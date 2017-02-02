
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>



void activate_keyboard();
void deactivate_keyboard();

@interface FPViewController : GLKViewController <GLKViewControllerDelegate>
{
@public
};


-(void) checkScreenParms;

@end
