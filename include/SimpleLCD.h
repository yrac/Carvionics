/**
 * SimpleLCD - Carvionics EFIS Avionics Display Library
 * 
 * OOP-based cockpit display system for Speeduino engine monitoring
 * Includes:
 * - ECUData: Engine parameter storage
 * - SpeeduinoParser: Binary frame serial decoder
 * - SyncManager: Sync loss & recovery state machine
 * - DisplayManager: TFT driver abstraction
 * - UIStateMachine: Rendering orchestration
 * - UIScreen: EFIS-style UI rendering
 */

#ifndef SIMPLE_LCD_H
#define SIMPLE_LCD_H

// Include all OOP components
#include "ECUData.h"
#include "SpeeduinoParser.h"
#include "SyncManager.h"
#include "DisplayManager.h"
#include "UIStateMachine.h"
#include "UIScreen.h"

#endif
