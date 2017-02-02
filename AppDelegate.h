//
//  AppDelegate.h
//  FreePlay
//
//  Created by Kimber Leigh Whyte on 2/14/15.
//  Copyright (c) 2015 Musical DNA Software, LLC. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "FPViewController.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate>

{
@public
    	FPViewController	*glViewController;
}
@property (strong, nonatomic) UIWindow *window;

- (void) MIDI_scanExistingDevices;

@property (nonatomic, retain) FPViewController *glViewController;

double getcurrenttime();
@end

