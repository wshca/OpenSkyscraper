#include "stairs.h"

using namespace OSS;
using namespace Classic;


ItemDescriptor StairsItem::descriptor = {
	kStairsType,
	kStructureGroup,
	kTransportCategory,
	1,
	(0),
	5000,
	int2(8, 2),
	int2(8, 2),
	rectmaski(new recti(0, 0, 4, 1), new recti(4, 1, 4, 1), NULL)
};





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Initialization
//----------------------------------------------------------------------------------------------------

StairsItem::StairsItem(Tower * tower) : StairslikeItem(tower, &descriptor)
{
	numAnimationFrames = 14;
	numAnimationFramesPerTexture = 7;
	baseTextureName = "simtower/transport/stairs";
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Basic Sprites
//----------------------------------------------------------------------------------------------------

void StairsItem::updateBackground()
{
	StairslikeItem::updateBackground();
	
	//Reduce the upper background's height, since the texture accounts for a ceiling that isn't
	//actually there.
	backgrounds[1]->rect.size.y -= 12;
}
