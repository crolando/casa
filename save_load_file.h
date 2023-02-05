#ifndef save_load_file_h
#define save_load_file_h

/*
*  Functions to save and load plano graphs.
*/

#include "plano_api.h"
#include <string>
#include <fstream>

// Load & Save States
struct plano_state_flags {
    bool waiting_on_os_save_dialog = false;   // true when an asyncronous, native file browser is visible to the user. Dialog is geared towards saving a project file.
    bool modal_save_challange_load = false;   // true when the "do you want to save before you load" modal is visible. Triggered when "load" is requested with a dirty project.
    bool waiting_on_os_load_dialog = false;   // true when an asyncronous, native file browser is visible to the user. Dialog is geared towards loading a project file.
    bool modal_save_challange_new = false;    // true when the "do you want to save before you create a new project" modal is visible. Triggered when "new" is requested with a dirty project.
    bool waiting_on_new = false;              // true when a new project needs to be created.  This is a little silly but it re-uses the pattern for the other project handlers.
    bool modal_save_challange_quit = false;   // true when the "do you want to save before you quit" modal is visible. Triggered when "quit" is requested with a dirty project.
    bool waiting_on_quit = false;             // true when a quit is requested by the menu.  Tracking it this way allows us to handle edge cases like when a user goes to quit then indecisively cancels a save challange.
    bool done = false;
    plano::types::ContextData* context_a = nullptr;
    std::string last_save_file_address = "";
};

// Load project function
// This is "stateful": meaning you must first create a nodos context, and set the context
// Loading will then affect the currently set context.
void load_project_file(const char* file_address);

// Save project function
// This is "stateful": meaning you must first create a nodos context, and set the context
// Saving will then affect the currently set context.
int save_project_file(const char* file_address);

void handle_load_save_dialogs(plano_state_flags& pstate, const plano::types::ContextCallbacks& cbk);

#endif /* save_load_file_h */
