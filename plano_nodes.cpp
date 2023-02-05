#include "node_defs/blueprint_demo.h"
#include "node_defs/import_animal.h"
#include "node_defs/widget_demo.h"
#include "node_defs/plano_nodes.h"


void RegiserNodesToActiveContext(void) {
// Register node types to the context that is "active"
plano::api::RegisterNewNode(node_defs::blueprint_demo::InputActionFire::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::blueprint_demo::OutputAction::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::blueprint_demo::Branch::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::blueprint_demo::DoN::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::blueprint_demo::SetTimer::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::blueprint_demo::SingleLineTraceByChannel::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::blueprint_demo::PrintString::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::import_animal::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::widget_demo::BasicWidgets::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::widget_demo::TreeDemo::ConstructDefinition());
plano::api::RegisterNewNode(node_defs::widget_demo::PlotDemo::ConstructDefinition());
}
