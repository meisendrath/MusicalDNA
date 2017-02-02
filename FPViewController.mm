
#import "FPViewController.h"

#import "LElement.h"

#import "FreePlayDispatcher.h"
#import "Application.h"
#import "AppDelegate.h"


extern double curtimeval;
extern double curtimeval_p;
extern float gDeviceScale, gScreenAspectRatio;
extern int gScreenWidth, gScreenHeight;
extern CGRect gWindowRect;
extern int gGameState;
extern bool background_state;		// app has been told it's in the background, throttle down the 3D framerate
extern id gGLcontext;

extern AppDelegate *the_delegate;

extern double getcurrenttime();

// keyboard

bool keyboard_can_resign = true;
bool keyboard_self_dismissed = false;
std::string keyboard_string;
GLKView *the_view;
void activate_keyboard() {
    keyboard_can_resign = false;
    keyboard_self_dismissed = false;
    [the_view becomeFirstResponder];
}
void deactivate_keyboard() {
    keyboard_can_resign = true;
    [the_view resignFirstResponder];
}

//

@interface FPViewController () {
}
@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;
@end

@implementation FPViewController

- (void)dealloc
{
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }
    
    // Dispose of any resources that can be recreated.
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
    
    //    self.effect = nil;
}


#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    
    float time_passed = (float)self.timeSinceLastUpdate;
    if (time_passed > 0.1f) time_passed = 0.1f;
    if (time_passed < 0.0f) time_passed = 0.0f;
    curtimeval += time_passed;				// the only place curtimeval gets advanced
    curtimeval_p += time_passed;
    
    if (!background_state) {
        
        if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 8.0f) {		// iOS 8
        } else {
            
            
            // update gScreenWidth/gScreenHeight/gScreenAspectRatio
            {
                CALayer *ly = self.view.layer;
                int sw = (int)([ly bounds].size.width*gDeviceScale);
                int sh = (int)([ly bounds].size.height*gDeviceScale);
                if ((gScreenWidth != sw) || (gScreenHeight != sh)) {
                    gScreenWidth = sw;
                    gScreenHeight = sh;
                    gScreenAspectRatio = ((float)gScreenWidth)/gScreenHeight;
                    
                    for (int scii=0; scii<LElement::gScene.size(); scii++) LElement::gScene[scii]->cleanup_on_exit();
                    LElement::gScene.remove_all();	// rotated screen - clear out gScene before GAMESTATE_MESSAGE_ENTER
                    free_play_dispatcher(gGameState, GAMESTATE_MESSAGE_ENTER, getcurrenttime(), 1);		// arg1 is 1 to indicate that this enter came from a rotation, not a state change
                }
            }
        }	// <8.0
        
        
        // UPDATE THE REST OF THE GAME!!!
        if (!shellUpdateScene(CFAbsoluteTimeGetCurrent()))
            printf("shellUpdateScene error\n");
        
        
    }
}

-(void) checkScreenParms {
    CGRect	bnds = [[UIScreen mainScreen] bounds];
    gScreenWidth = (int)(bnds.size.width*gDeviceScale);
    gScreenHeight = (int)(bnds.size.height*gDeviceScale);
    
    
    if (self.view != nil) self.view.bounds = self.view.frame = CGRectMake(0, 0, bnds.size.width, bnds.size.height);
    the_delegate.window.bounds = the_delegate.window.frame = CGRectMake(0, 0, bnds.size.width*gDeviceScale, bnds.size.height*gDeviceScale);
}

-(void) viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
    printf("viewWillTransitionToSize:%g,%g withTransitionCoordinator\n", size.width, size.height);
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    
    [coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext> context) {
        
        [self checkScreenParms];
        
        gScreenAspectRatio = ((float)gScreenWidth)/gScreenHeight;
        for (int scii=0; scii<LElement::gScene.size(); scii++) LElement::gScene[scii]->cleanup_on_exit();
        LElement::gScene.remove_all();	// rotated screen - clear out gScene before GAMESTATE_MESSAGE_ENTER
        free_play_dispatcher(gGameState, GAMESTATE_MESSAGE_ENTER, getcurrenttime(), 1);		// arg1 is 1 to indicate that this enter came from a rotation, not a state change
        
        
    } completion:^(id<UIViewControllerTransitionCoordinatorContext> context) {
    }];
}

- (void)glkViewControllerUpdate:(GLKViewController *)viewController
{
    //
    printf("glkViewControllerUpdate\n"); // APPARENTLY NEVER CALLED
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    if (background_state) return;
    
    if (!shellRenderScene())
        printf("shellRenderScene error\n");
        
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    //    return (interfaceOrientation == UIInterfaceOrientationPortrait);
    return YES;
}

- (BOOL)shouldAutorotate
{
    return YES;
}

-(NSUInteger)supportedInterfaceOrientations{
    return UIInterfaceOrientationMaskAll;
}


@end

