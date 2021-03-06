#include "construction.h"

#include "../../items/item.h"

using namespace OSS;
using namespace Classic;





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Construction
//----------------------------------------------------------------------------------------------------

ConstructionTool::ConstructionTool(ToolsUI * ui) : Tool(ui),
updateTemplateRectIfNeeded(this, &ConstructionTool::updateTemplateRect, &updateIfNeeded)
{	
	//Select the lobby item by default
	setItemType(kLobbyType);
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Item
//----------------------------------------------------------------------------------------------------

ItemType ConstructionTool::getItemType()
{
	return itemType;
}

void ConstructionTool::setItemType(ItemType type)
{
	if (itemType != type) {
		itemType = type;
		setItemDescriptor(Item::descriptorForItemType(type));
		ui->ui->sendEvent(new Event(Event::kToolChanged));
	}
}

ItemDescriptor * ConstructionTool::getItemDescriptor()
{
	return itemDescriptor;
}

void ConstructionTool::setItemDescriptor(ItemDescriptor * descriptor)
{
	if (itemDescriptor != descriptor) {
		itemDescriptor = descriptor;
		updateTemplateRectIfNeeded.setNeeded();
	}
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Template
//----------------------------------------------------------------------------------------------------

double2 ConstructionTool::getTemplateCenter()
{
	return templateCenter;
}

void ConstructionTool::setTemplateCenter(double2 center)
{
	if (templateCenter != center) {
		templateCenter = center;
		updateTemplateRectIfNeeded.setNeeded();
	}
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark State
//----------------------------------------------------------------------------------------------------

void ConstructionTool::update()
{
	//Update the template rect if required
	updateTemplateRectIfNeeded();
}

void ConstructionTool::updateTemplateRect()
{
	//Set the template's size
	ItemDescriptor * desc = getItemDescriptor();
	if (desc) {
		if (desc->attributes & kFlexibleWidthAttribute)
			templateRect.size = desc->minUnit;
		else
			templateRect.size = desc->cells;
	}
	
	//In order to position the template, we have to convert the template center which is in world
	//coordinates to cell coordinates. The first step is to convert the template rect's size to
	//world coordinates so we can do the centering properly.
	double2 size = ui->getTower()->structure->cellToWorld(templateRect.size);
	
	//Now calculate the origin of the template in world coordinates.
	double2 origin = templateCenter - (size / 2);
	
	//Now convert the origin to cell coordinates.
	templateRect.origin = ui->getTower()->structure->worldToCell(origin);
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Drawing
//----------------------------------------------------------------------------------------------------

void ConstructionTool::draw(rectd dirtyRect)
{
	if (!getItemDescriptor()) return;
	
	//Draw the template rect. To do this we first need to convert the template rect to world co-
	//ordinates.
	rectd world = ui->getTower()->structure->cellToWorld(templateRect);
	
	//Since we're drawing in the UI and everything is in window coordinates, we have to convert the
	//world coordinates to window coordinates.
	rectd window = ui->getScene()->worldToWindow(world);
	
	//Now we have to fix the window rect. Remember that we want to draw its frame and since drawing
	//a line requires the center of a pixel instead of a corner of it, we have to inset the entire
	//rect by 0.5 (half a pixel).
	window.inset(double2(0.5, 0.5));
	
	//Now we may draw the frame around the rect
	glColor4f(1, 1, 1, 0.85);
	Texture::unbind();
	glBegin(GL_LINE_STRIP);
	glVertex2d(window.minX(), window.minY());
	glVertex2d(window.maxX(), window.minY());
	glVertex2d(window.maxX(), window.maxY());
	glVertex2d(window.minX(), window.maxY());
	glVertex2d(window.minX(), window.minY());
	glEnd();
	
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Event Handling
//----------------------------------------------------------------------------------------------------

bool ConstructionTool::eventMouseDown(MouseButtonEvent * event)
{
	if (!getItemDescriptor())
		return false;
	
	TowerStructure::ConstructionResult result;
	
	//We need to make sure that the template rect has been updated
	updateTemplateRect();
	
	//If we've selected a flexible width item, we have to start buildling
	if (getItemDescriptor()->attributes & kFlexibleWidthAttribute) {
		
		//Keep track of where we started construction
		initialTemplateRect = templateRect;
		
		//Do the initial build
		result = ui->getTower()->structure->constructItem(getItemDescriptor(), templateRect,
														  initialTemplateRect);
		
		//If the inital build was successful, we start dragging and halt the constructions.
		if (result.success) {
			isDraggingConstruction = true;
			ui->getTower()->structure->setConstructionsHalted(true);
		}
	}
	
	//Otherwise we have a fixed width item selected, which is trivial to construct
	else {
		result = ui->getTower()->structure->constructItem(getItemDescriptor(), templateRect,
														  templateRect);
	}
	
	//If we weren't able to build the item, play the annoying "click"
	if (!result.success) {
		OSSObjectError << result.failureReason << std::endl;
		Audio::shared()->play(Sound::named("simtower/construction/impossible"),
										  SoundEffect::kTopLayer);
	}
	
	return true;
}

bool ConstructionTool::eventMouseUp(MouseButtonEvent * event)
{
	if (!getItemDescriptor() || !isDraggingConstruction)
		return false;
	
	//End any dragging construction going on
	isDraggingConstruction = false;
	
	//Re-enable the constructions
	ui->getTower()->structure->setConstructionsHalted(false);
	
	//TODO: resume constructions of the tower
	return true;
}

bool ConstructionTool::eventMouseMove(MouseMoveEvent * event)
{
	if (!getItemDescriptor())
		return false;
	
	//Set the template's center to be where the mouse is. First we have to convert the mouse po-
	//sition which is in window coordinates to world coordinates.
	double2 center = ui->getScene()->windowToWorld(event->position);
	
	//Then we have to make sure that the center doesn't change its Y coordinate during dragging con-
	//struction.
	if (isDraggingConstruction)
		center.y = getTemplateCenter().y;
	
	//Finally we set the new center
	setTemplateCenter(center);
	
	
	//If we're dragging a construction, we have to construct the item under the new conditions
	if (isDraggingConstruction) {
		
		//Make sure the template rect is up to date, since we just changed the template center whose
		//changes may not yet have applied to the template.
		updateTemplateRect();
		
		//Build, it's that simple
		ui->getTower()->structure->constructItem(getItemDescriptor(), templateRect,
												 initialTemplateRect);
	}
	
	return true;
}
