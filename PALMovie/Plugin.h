//
// Plugin.h
//
//
// [~ 20 01 2000 ~]
// [~ 01 02 2000 ~]	Flipping support, yeah!

#define FIELD_W 768				// field width (in pixels (shorts))
#define FIELD_H 288				// field height (in lines)

// ----------------------------------------------------------------------------
// PLUGINPROPS FLAGS

#define PLUGINF_HAVECONFIG		0x00000001
// Plugin have own custom config and wants to use it
// This flag enables button in F7 (named [Config])
// When user presses that button 'plugin_Config()' is called...
// In most cases the params you get in the PLUGINPARAMS struct
// will be sufficient. If you need to do some extra-special-magic config
// this is the way to do that.
// Even if you don't need any own config, it would be nice if you can explain
// which param in F7 does what in plugin. Look at the example...

#define PLUGINF_WANTSFLIPPING	0x00000002
// This plugin wants to use page flipping.
// When you set this flag you will be drawing on some offscreen buffer
// which will not be visible until you return RET_FLIPME in 'plugin_DoField()'
// NOTE that you are not able to detect whether flipping is enabled or not.
// If it's not you will get the front surface to copy data to.
// Anyway you shouldn't make any assumptions about that...
// NOTE2 If you use flipping you have to fill whole frame (even & odd lines).
// If you don't you will get some nasty/ugly visual effects.

// ----------------------------------------------------------------------------
// RETURN FLAGS FOR 'plugin_DoField()'

#define RET_FLIPME				0x00000001
// Flip frames. Will do nothing if flipping is disabled.


// ----------------------------------------------------------------------------
typedef struct
{
	char *shortname;		// short name to display in combo
	char *longname;			// long name to display in static
	long flags;				// flags as described
} PLUGINPROPS;

typedef struct
{
	unsigned short **src;		// this frame
	unsigned short **lastsrc;	// last frame
	unsigned short **desteven;	// destination even lines
	unsigned short **destodd;	// destination odd lines
	int level1;					// 0 <-> 255 as in the F7
	int level2;					// 0 <-> 255 as in the F7
	int x;						// -15 <-> 15 as in the F7
	int y;						// -5 <-> 5 as in the F7
	int key1;					// 0/1 as in the F7
	int key2;					// 0/1 as in the F7
	int odd;					// 0/1
} PLUGINPARAMS;

