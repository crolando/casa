#include "save_load_file.h"
#include "tinyfiledialogs.h"
#include "node_defs/plano_nodes.h"

int save_project_file(const char* file_address)
{
    size_t save_size;
    char* cbuffer = plano::api::SaveNodesAndLinksToBuffer(&save_size);
    auto ofs = std::ofstream(file_address);
    ofs << std::string(cbuffer);
    return 0;
}

void load_project_file(const char* file_address)
{
    assert(plano::api::GetContext() != nullptr); // Context was not empty before a load. Set context to null or destroy the old context first.

    // Load the project file
    std::ifstream inf(file_address);
    std::stringstream ssbuf;
    ssbuf << inf.rdbuf();
    std::string sbuf = ssbuf.str();
    size_t load_size = sbuf.size();
    plano::api::LoadNodesAndLinksFromBuffer(load_size, sbuf.c_str());

}

// Thread stuff
#include <thread>
#include <future>
// Deals with the situation where a "OS dialog (save file, load file)" blocks
// the draw thread.
template<typename R>
bool is_ready(std::future<R> const& f)
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

// handle load/save child processes
std::future<char*> load_file_future, save_file_future;
static const char* filter_extensions[1] = { "*.csa" };


void handle_load_save_dialogs(plano_state_flags& pstate, const plano::types::ContextCallbacks& cbk)
{
    // Save dialog handling.  This spawns a new window, so it does some parallel processing stuff
    // Save / load behavior is complex to handle the case where you choose "load", but you have unsaved changes.
    if (pstate.waiting_on_os_save_dialog)
    {
        // Spawn the dialog if this is the first frame where (waiting_on_os_save_dialog == true)
        if (!save_file_future.valid())
        {
            save_file_future = std::async(tinyfd_saveFileDialog, "Save the Casa Project", nullptr, 1, filter_extensions, "Casa Project");
        }
        // Wait until the save dialog interactions are done.
        if (is_ready(save_file_future))
        {
            const char* save_file = save_file_future.get();
            if (save_file) {
                save_project_file(save_file);
                plano::api::ClearProjectDirtyFlag();
                pstate.last_save_file_address = std::string(save_file);
            }
            else {
                // Save was cancelled in the save dialog

                // Safety - do not quit immediately after cancelling a save dialog during a modal_save_challange_quit
                if (pstate.waiting_on_quit)
                    pstate.waiting_on_quit = false;
            }
            // Book keeping
            ImGui::CloseCurrentPopup();
            pstate.waiting_on_os_save_dialog = false;
            if (pstate.modal_save_challange_load) {
                pstate.modal_save_challange_load = false;  // No longer  
            }

            if (pstate.modal_save_challange_new) {
                ImGui::CloseCurrentPopup();
                pstate.modal_save_challange_new = false;
            }

        }
    }

    // Load dialog handling.  This spawns a new window, so it does some parallel processing stuff
    if (pstate.waiting_on_os_load_dialog && !pstate.waiting_on_os_save_dialog)
    {
        if (!load_file_future.valid())
        {
            load_file_future = std::async(tinyfd_openFileDialog, "Load the Casa Project", nullptr, 1, filter_extensions, "Casa Project", 0);
        }
        if (is_ready(load_file_future))
        {
            const char* load_file = load_file_future.get();
            if (load_file) {
                if (pstate.context_a != nullptr)
                {
                    plano::api::DestroyContext(pstate.context_a);
                }
                pstate.context_a = plano::api::CreateContext(cbk, "../plano/data/");
                plano::api::SetContext(pstate.context_a);
                RegiserNodesToActiveContext();
                load_project_file(load_file);
            }
            else {
                ; // load cancelled in UI
            }
            // Book keeping
            pstate.waiting_on_os_load_dialog = false;
        }
    }

    // New handling.  This creates a new project.
    if (pstate.waiting_on_new && !pstate.waiting_on_os_save_dialog)
    {
        pstate.context_a = plano::api::CreateContext(cbk, "../plano/data/");
        plano::api::SetContext(pstate.context_a);
        RegiserNodesToActiveContext();

        // Book keeping
        pstate.waiting_on_new = false;
    }

    // Quit handling.  This quits.
    if (pstate.waiting_on_quit && !pstate.waiting_on_os_save_dialog)
    {
        pstate.done = true;
        // Book keeping
        pstate.waiting_on_quit = false;
    }
}

void handle_menu_state(plano_state_flags& pstate)
{
    // Draw modal window when the user chooses "load", but has unsaved changes.
// handle menu popups after menus are closed.
// see https://github.com/ocornut/imgui/issues/331
    if (pstate.modal_save_challange_load)
    {
        pstate.modal_save_challange_load = false;
        ImGui::OpenPopup("modal_save_challange_load");
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
    if (ImGui::BeginPopupModal("modal_save_challange_load", NULL, ImGuiWindowFlags_None))
    {
        ImGui::Text("Before you load a new project, would you like to save the current changes?");
        if (ImGui::Button("Save First"))
        {
            // The logic for this is kind of spaghetti.  This will draw this frame, but will hit the save
            // before the next frame.  The save always happens before loads, so it will chain the load
            // after completing the save.
            // Note that if waiting_on_save_dialog == true too, this will chain save  + load sequentially.
            pstate.waiting_on_os_save_dialog = true; // This will draw this frame, but will hit the save before the next frame.  
            pstate.waiting_on_os_load_dialog = true;
            ImGui::CloseCurrentPopup(); // close challange
        }
        if (ImGui::Button("Ditch Project"))
        {
            pstate.waiting_on_os_load_dialog = true; // strictly not needed.
            pstate.waiting_on_os_save_dialog = false;
            ImGui::CloseCurrentPopup(); // close challange
        }
        ImGui::EndPopup();
    }

    // Draw modal window when the user chooses "new", but has unsaved changes.
    // handle menu popups after menus are closed.
    // see https://github.com/ocornut/imgui/issues/331
    if (pstate.modal_save_challange_new)
    {
        pstate.modal_save_challange_new = false;
        ImGui::OpenPopup("modal_save_challange_new");
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
    if (ImGui::BeginPopupModal("modal_save_challange_new", NULL, ImGuiWindowFlags_None))
    {
        ImGui::Text("Before you create a new project, would you like to save the current changes?");
        if (ImGui::Button("Save First"))
        {
            // The logic for this is kind of spaghetti.  This will draw this frame, but will hit the save
            // before the next frame.  The save always happens before loads, so it will chain the load
            // after completing the save.
            pstate.waiting_on_os_save_dialog = true;
            pstate.waiting_on_new = true;
            ImGui::CloseCurrentPopup(); // close challange
        }
        if (ImGui::Button("Ditch Project"))
        {
            pstate.waiting_on_new = true; // strictly not needed.
            pstate.waiting_on_os_save_dialog = false;
            ImGui::CloseCurrentPopup(); // close challange
        }
        ImGui::EndPopup();
    }

    // Draw modal window when the user chooses "quit", but has unsaved changes.
    // handle menu popups after menus are closed.
    // see https://github.com/ocornut/imgui/issues/331
    if (pstate.modal_save_challange_quit)
    {
        pstate.modal_save_challange_quit = false;
        ImGui::OpenPopup("modal_save_challange_quit");
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
    if (ImGui::BeginPopupModal("modal_save_challange_quit", NULL, ImGuiWindowFlags_None))
    {
        ImGui::Text("Before you quit, would you like to save the current changes?");
        if (ImGui::Button("Save First"))
        {
            // The logic for this is kind of spaghetti.  This will draw this frame, but will hit the save
            // before the next frame.  The save always happens before loads, so it will chain the load
            // after completing the save.
            pstate.waiting_on_os_save_dialog = true;
            pstate.waiting_on_quit = true;
            ImGui::CloseCurrentPopup(); // close challange
        }
        if (ImGui::Button("Ditch Project"))
        {
            pstate.waiting_on_quit = true; // strictly not needed.
            pstate.waiting_on_os_save_dialog = false;
            ImGui::CloseCurrentPopup(); // close challange
        }
        ImGui::EndPopup();
    }


}
