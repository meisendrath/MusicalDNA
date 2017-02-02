//
//  AppDelegate.m
//  FreePlay
//
//  Created by Kimber Leigh Whyte on 2/14/15.
//  Copyright (c) 2015 Musical DNA Software, LLC. All rights reserved.
//

#import "AppDelegate.h"
#include <Foundation/Foundation.h>
#include <Foundation/NSDate.h>

#include <CoreMotion/CoreMotion.h>

#import "TTextureFile.h"

#import "HAL.h"

#import "LElement.h"
#include "FP_VISUALIZER.h"
#include "FP_ROUTING.h"
#import "FreePlayDispatcher.h"
#include "FPGameStatus.h"
#include "GameState.h"		// for ::send(msg)
#import "FPViewController.h"
#import "TouchGLKView.h"

#include "ios/MIDI.h"



float gDeviceScale, gScreenAspectRatio;
extern int gScreenWidth, gScreenHeight;
extern HAL *gHAL;
extern FPGameStatus gGS;
extern int gGameState;				// is this state or device?
std::string version_string;
bool app_can_background;	//UIApplicationExitsOnSuspend
extern TMatrix gOrientationMatrix;
extern CFTimeInterval curtimeval_p;
extern GLKView *the_view;


extern void report_memory();
extern "C" int bg_work_time_slice();

AppDelegate *the_delegate = NULL;
static CMMotionManager *motionManager;
bool background_state = false;		// app has been told it's in the background, throttle down the 3D framerate
double appl_start_time;
id gGLcontext;
CGRect gWindowRect;



// accumulate in a global string, output when a \n is seen (but don't output the \n!!)
static std::string gPendingDebugOutput;
void logprintf(const char* fmt, ...) {
    int size = 100;
    std::string str;
    va_list vl;
    while (1) {
        str.resize(size);
        va_start(vl, fmt);
        int nsize = vsnprintf((char *)str.c_str(), size, fmt, vl);
        va_end(vl);
        if ((nsize > -1) && (nsize < size)) {
            str.resize(nsize);
            break;
        }
        if (nsize > -1) {
            size = nsize + 1;
        } else {
            size *= 2;
        }
    }
    gPendingDebugOutput += str;
    
    std::string s;
    while (true) {
        if (gPendingDebugOutput.size() == 0) break;
        char c = gPendingDebugOutput[0];
        gPendingDebugOutput.erase(gPendingDebugOutput.begin());
        // accumlate s up to next \n IF NONE, BREAK
        if (c == '\n') {
#ifdef _DEBUG
            NSLog(@"%@", [NSString stringWithCString:s.c_str() encoding:NSASCIIStringEncoding]);	// can't use printf() here!!!
#endif
            s.resize(0);
        } else {
            s += c;
        }
    }
    gPendingDebugOutput = s;
}



@implementation AppDelegate

@synthesize glViewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    appl_start_time = CFAbsoluteTimeGetCurrent();
    
    CGRect	bds = [[UIScreen mainScreen] bounds];
    
    float vers = [[[UIDevice currentDevice] systemVersion] floatValue];
    int w = 320, h = 480;
    if (vers >= 3.2f) {
        UIScreen* mainscr = [UIScreen mainScreen];
        w = (int)mainscr.currentMode.size.width;
        h = (int)mainscr.currentMode.size.height;
    }
    
    gDeviceScale = (float)[UIScreen mainScreen].scale;
    
    if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0f) {		// iOS 8: starting up in landscape mode was screwy
        w = (int)(bds.size.width * gDeviceScale);
        h = (int)(bds.size.height * gDeviceScale);
    }
    
    printf("VERSION=%g SCREEN SIZE=%d,%d %sbounds=[%g,%g,%g,%g] scale=%g orientation=%ld\n", vers, w, h, (gDeviceScale >= 2.0f)?"RETINA DISPLAY ":"", bds.origin.x, bds.origin.y, bds.size.width, bds.size.height, gDeviceScale, (long)[[UIDevice currentDevice] orientation]);
    
    gScreenWidth = w;
    gScreenHeight = h;
    gScreenAspectRatio = ((float)gScreenWidth)/gScreenHeight;
    
#ifdef DEBUG_SHOW_LOADING_DETAIL
    printf("applicationDidFinishLaunching...\n");
    report_memory();
#endif // DEBUG_SHOW_LOADING_DETAIL
    
    
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    int ver = CFBundleGetVersionNumber(mainBundle);
    NSString *sv = (NSString *)CFBundleGetValueForInfoDictionaryKey(mainBundle, (CFStringRef)@"CFBundleShortVersionString");
    int ver3 = (ver & 0x0000f000) >> 12;
    int ver4 = (ver & 0x000000ff);
    char vs[20];
    size_t ivs = 0;
    sprintf(vs, "%s", [sv UTF8String]);
    ivs = strlen(vs);
    if (ver4 > 0) {
        switch (ver3) {
            case 2:		vs[ivs++] = 'd';		break;
            case 4:		vs[ivs++] = 'a';		break;
            case 6:		vs[ivs++] = 'b';		break;
            case 8:		vs[ivs++] = 'f';		break;
            default:	vs[ivs++] = '.';		break;
        }
        sprintf(vs+ivs, "%03d", ver4);
        ivs = strlen(vs);
    }
    vs[ivs++] = '\0';
    //

    version_string = vs;
    printf("version_string='%s'\n", vs);
    
    
    //extern bool app_can_background;//UIApplicationExitsOnSuspend
    const void *sus = CFBundleGetValueForInfoDictionaryKey(mainBundle, (CFStringRef)@"UIApplicationExitsOnSuspend");
    
    
    if (sus != NULL) {
        //CFTypeID type = CFGetTypeID(sus);
        //if (CFArrayGetTypeID() == type)
        //    printf("sus is an array.\n");
        //else
        //    printf("sus is NOT an array. %ld\n", type);
        
        app_can_background = true;
        NSNumber *nm = (NSNumber *)(sus);
        if (nm != NULL) {
            app_can_background = [nm intValue] == 0;
            //printf("nm = %d\n", [nm intValue]);
        }
    }
    
    gHAL = HAL::initial_setup();
    
    gGLcontext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    [EAGLContext setCurrentContext:gGLcontext];
    
    //	[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    
    
  
    
   
    
    // create a full-screen window
    if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0f) {		// iOS 8
        _window = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, gScreenWidth, gScreenHeight)];
    } else {
        float md = fmaxf(gScreenHeight, gScreenWidth);
        _window = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, md, md)];
#warning square window prevents touches from being ignored (when touch is near the home key)
    }
    gWindowRect = [_window frame];

    // create the OpenGL view and add it to the window
    glViewController = [[FPViewController alloc] init];
    [glViewController checkScreenParms];
    
    
    // show the window
    [_window addSubview:glViewController.view];
    [_window makeKeyAndVisible];
    //    _window.rootViewController = glViewController;
    if ([[UIDevice currentDevice].systemVersion floatValue] < 6.0) {
        // how the view was configured before IOS6
        _window.rootViewController = glViewController;
    } else {
        // this is the code that will start the interface to rotate once again
        [_window setRootViewController: glViewController];
    }
    
    
    
        glViewController.view = [[TouchGLKView alloc] initWithFrame:CGRectMake(0, 0, gScreenWidth, gScreenHeight) context:gGLcontext];
    
    the_view = (GLKView *)glViewController.view;
    the_view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    the_view.multipleTouchEnabled = YES;
    
    glViewController.view.contentMode = UIViewContentModeScaleToFill;

    gWindowRect = [_window frame];
    [glViewController checkScreenParms];
    
    
    if ([[[UIDevice currentDevice] systemVersion] floatValue] < 8.0f) {		// before iOS 8
        CALayer *ly = the_view.layer;//[the_view.layer presentationLayer];
        int sw, sh;

        sw = (int)([ly bounds].size.width*gDeviceScale);
        sh = (int)([ly bounds].size.height*gDeviceScale);
        
        if ((gScreenWidth != sw) || (gScreenHeight != sh)) {
            gScreenWidth = sw;
            gScreenHeight = sh;
            gScreenAspectRatio = ((float)gScreenWidth)/gScreenHeight;
        }
        
        UIInterfaceOrientation iOrientation = [UIApplication sharedApplication].statusBarOrientation;
        
        if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0f) {		// iOS 8
        } else {
            switch (iOrientation) {
                case UIInterfaceOrientationPortrait:									break;
                case UIInterfaceOrientationPortraitUpsideDown:		the_view.transform = CGAffineTransformMakeRotation((float)M_PI);		break;
                case UIInterfaceOrientationLandscapeLeft:			the_view.transform = CGAffineTransformMakeRotation(-(float)M_PI/2);		break;
                case UIInterfaceOrientationLandscapeRight:			the_view.transform = CGAffineTransformMakeRotation((float)M_PI/2);		break;
            }
            switch (iOrientation) {
                case UIInterfaceOrientationPortrait:									break;
                case UIInterfaceOrientationPortraitUpsideDown:							break;
                case UIInterfaceOrientationLandscapeLeft:
                case UIInterfaceOrientationLandscapeRight:
                    //printf("    screen size swapped to: %dx%d\n", sh, sw);
                    gScreenWidth = sh;
                    gScreenHeight = sw;
                    gScreenAspectRatio = ((float)gScreenWidth)/gScreenHeight;
                    glViewController.view.bounds = CGRectMake(0, 0, gScreenHeight/gDeviceScale, gScreenWidth/gDeviceScale);
                    glViewController.view.frame = CGRectMake(0, 0, gScreenHeight/gDeviceScale, gScreenWidth/gDeviceScale);
                    break;
            }
        }
    }
    
    free_play_dispatcher(gGameState, GAMESTATE_MESSAGE_ENTER, CFAbsoluteTimeGetCurrent());
    
//    [self setup_sound];
    
    the_delegate = self;
    MIDI_didFinishLaunchingWithOptions();
    
    
    
    
    UIAccessibilityRegisterGestureConflictWithZoom();
    
    motionManager = [[CMMotionManager alloc] init];
    motionManager.deviceMotionUpdateInterval = 0.02; // 50 Hz
    [motionManager startDeviceMotionUpdates];

    
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

double getcurrenttime() {
    return CFAbsoluteTimeGetCurrent();
}

- (void) MIDI_scanExistingDevices {
    MIDI_scanExistingDevices();
}

@end

