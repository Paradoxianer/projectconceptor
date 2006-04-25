#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/ScrollView.h>
#include <support/List.h>

#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>

#include <string.h>

#include "ToolItem.h"

#include "NavigatorEditor.h"
#include "PCommandManager.h"
#include "MessageListView.h"

#include "NodeItem.h"


NavigatorEditor::NavigatorEditor():PEditor(),BView(BRect(0,0,200,200),"NavigatorEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW|B_NAVIGABLE|B_NAVIGABLE_JUMP)
{
	Init();
	BView::SetDoubleBuffering(1);
//	SetDrawingMode(B_OP_ALPHA);
}

void NavigatorEditor::Init(void)
{
	insertCommand	= NULL;
	selectCommand	= NULL;
	deleteCommand	= NULL;
	connectCommand	= NULL;
	renderString	= new char[30];
	font_family		family;
	font_style		style;

/*	BMessage		*dataMessage	= new BMessage();
	dataMessage->AddString("Name","Untitled");
	//preparing the standart ObjectMessage
	nodeMessage	= new BMessage(P_C_CLASS_TYPE);
	nodeMessage->AddMessage("Data",dataMessage);
	//Preparing the standart FontMessage
	fontMessage		= new BMessage();
	fontMessage->AddInt8("Encoding",be_plain_font->Encoding());
	fontMessage->AddInt16("Face",be_plain_font->Face());
	be_plain_font->GetFamilyAndStyle(&family,&style);
	fontMessage->AddString("Family",(const char*)&family);
	fontMessage->AddInt32("Flags", be_plain_font->Flags());
	fontMessage->AddFloat("Rotation",be_plain_font->Rotation());
	fontMessage->AddFloat("Shear",be_plain_font->Shear());
	fontMessage->AddFloat("Size",be_plain_font->Size());
	fontMessage->AddInt8("Spacing",be_plain_font->Spacing());
	fontMessage->AddString("Style",(const char*)&style);
	rgb_color	fontColor			= {111, 151, 181, 255};
	fontMessage->AddRGBColor("Color",fontColor);
	
	//perparing Pattern Message
	patternMessage	=new BMessage();
	//standart Color 
	rgb_color	fillColor			= {152, 180, 190, 255};
	patternMessage->AddRGBColor("FillColor",fillColor);
	rgb_color	borderColor			= {0, 0, 0, 255};
	patternMessage->AddRGBColor("BorderColor",borderColor);
	patternMessage->AddFloat("PenSize",1.0);
	patternMessage->AddInt8("DrawingMode",B_OP_ALPHA);
	patternMessage->AddFloat("Scale",1.0);
	rgb_color	highColor			= {0, 0, 0, 255};
	patternMessage->AddRGBColor("HighColor",highColor);
	rgb_color 	lowColor			= {128, 128, 128, 255};
	patternMessage->AddRGBColor("LowColor",lowColor);
	patternMessage->AddData("Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH));*/
}

void NavigatorEditor::AttachedToManager(void)
{
	PCommandManager *commandManager = doc->GetCommandManager();
	insertCommand	= commandManager->GetPCommand("Insert");
	selectCommand	= commandManager->GetPCommand("Select");
	deleteCommand	= commandManager->GetPCommand("Delete");
	connectCommand	= commandManager->GetPCommand("Connect");
	sentTo			= new BMessenger(doc);
	id				=	manager->IndexOf(this);
	sprintf(renderString,"NavigatorEditor%ld::Renderer",id);

	//put this in a seperate function??
	BList		*allNodes		= doc->GetAllNodes();
	BList		*allConnections	= doc->GetAllConnections();
	BRect		rootrect		= Bounds();
	rootrect.right				= 200;
	root						= new NodeListView(rootrect,allNodes);
	BMessage *invoked 			= new BMessage(N_A_INVOKATION);
	invoked->AddPointer("ListView",root);
	root->SetInvocationMessage(invoked);
	root->SetTarget(this);
	AddChild(new BScrollView("root",root,B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true));
	SetViewColor(255,255,255,255);
	Invalidate();
}

void NavigatorEditor::DetachedFromManager(void)
{

}

BList* NavigatorEditor::GetPCommandList(void)
{
	//at the Moment we dont support special Commands :-)
	return NULL;
}


void NavigatorEditor::ValueChanged()
{
	BView 			*child 				= NULL;
	NodeListView	*nodeListView		= NULL;
	MessageListView	*messageListView	= NULL;
	root->ValueChanged();
	if ( child = ChildAt(1) )
	{
		while ( child )
		{
			messageListView = dynamic_cast<MessageListView *>(((BScrollView *)child)->Target());
			if (messageListView)
				messageListView->ValueChanged();
			child = child->NextSibling();
		}
	} 
}

void NavigatorEditor::SetDirty(BRegion *region)
{
	BView::Invalidate(region);
}

bool NavigatorEditor::IsFocus(void) const
{
	return	 BView::IsFocus();

}

void NavigatorEditor::MakeFocus(bool focus = true)
{
	BView::MakeFocus(focus);
}



void NavigatorEditor::KeyDown(const char *bytes, int32 numBytes)
{

}

void NavigatorEditor::KeyUp(const char *bytes, int32 numBytes)
{
}


void NavigatorEditor::MessageReceived(BMessage *message)
{
	message->PrintToStream();
	switch(message->what) 
	{
		case P_C_VALUE_CHANGED:
		{
			ValueChanged();
			break;
		}
		case N_A_INVOKATION:
		{
			BListView	*source	= NULL;
			if (message->FindPointer("ListView",(void **) &source) == B_OK)
				InsertNewList(source);
			break;
		}
		case P_C_EDITOR_SWITCHED_ACTIV:
		{
			ValueChanged();
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void NavigatorEditor::InsertNewList(BListView *source)
{
	int32			selection	= -1;
	BaseListItem	*item		= NULL;
	selection = source->CurrentSelection(0);
	if (selection>=0)
	{
		item =(BaseListItem *) source->ItemAt(selection);
		//vorher alle  überfüssigen Views löschen
		BView *sibling=source->Parent()->NextSibling();
		while (sibling != NULL)
		{
			RemoveChild(sibling);
			sibling=source->Parent()->NextSibling();
		}
		if (item->GetSupportedType() == P_C_CLASS_TYPE)
		{
			BRect		listrect		= Bounds();
			listrect.left				= source->Parent()->Frame().right+5;
			listrect.right				= listrect.left	+200;
			if (listrect.right > Bounds().right)
			{
				ResizeTo(listrect.right+B_V_SCROLL_BAR_WIDTH+5,Bounds().bottom);
			}
			BListView	*list			= new MessageListView(listrect,((NodeItem *)item)->GetNode());
			BMessage *invoked 			= new BMessage(N_A_INVOKATION);
			invoked->AddPointer("ListView",list);
			list->SetInvocationMessage(invoked);
			list->SetTarget(this);	
			AddChild(new BScrollView("root",list,B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true));
		}
		Invalidate();
	}
}

void NavigatorEditor::DeleteRenderObject(BMessage *node)
{
}

