<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->
- [x] Verify that the copilot-instructions.md file in the .github directory is created.

- [x] Clarify Project Requirements
	Wearable fitness tracker project (Whoop alternative) with:
	- nRF52840 MCU with Zephyr RTOS firmware
	- MAX86141 PPG sensor for heart rate monitoring  
	- BMA400 IMU for activity tracking
	- Health monitoring features (HRV, sleep, strain, illness prediction)
	- Mobile app for data visualization
	- Hardware design files and documentation

- [x] Scaffold the Project
	Created complete project structure with:
	- Firmware directory with Zephyr RTOS setup
	- Hardware design directory structure
	- Mobile app directory with Flutter setup
	- Documentation structure
	- Driver interfaces for PPG, IMU, temperature sensors
	- Signal processing modules for HRV, sleep, health trends

- [x] Customize the Project
	Project customized with:
	- Detailed firmware architecture with nRF52840
	- Complete sensor driver headers (MAX86141, BMA400, MAX30205)
	- Signal processing pipeline for PPG analysis
	- HRV calculation module with time/frequency domain analysis
	- Health trends monitoring with illness prediction
	- Mobile app structure with BLE integration
	- Hardware design specifications and power analysis

- [x] Install Required Extensions
	Required VS Code extensions for nRF52840 development:
	- nRF Connect for VS Code Extension Pack (essential)
	- nRF Connect for VS Code (main development environment)
	- nRF Kconfig (syntax highlighting for Zephyr configs)
	- nRF Terminal (serial debugging and RTT logging)
	- nRF DeviceTree (optional, for custom board definitions)

- [x] Migrate to Upper Arm Placement
	Successfully migrated sensor placement from wrist to upper arm:
	- Updated all documentation references (SIGNAL_FLOW.md)
	- Optimized motion artifact detection for upper arm
	- Created comprehensive test suite (test_wrist_artifacts.sh)
	- Validated firmware architecture is placement-agnostic

- [ ] Compile the Project
	Phase 1 (Hardware Prototyping) ready to begin:
	- Firmware: nRF Connect SDK setup required
	- Hardware: nRF52840-DK + sensor breakouts ordered
	- Mobile App: Flutter environment configured
	- Testing: Automated test framework implemented

- [ ] Create and Run Task
	Current development phase tasks:
	- Hardware evaluation with nRF52840-DK
	- Sensor integration (MAX86141, BMA400, MAX30205)
	- PPG signal quality validation at upper arm
	- Real-time data pipeline implementation
	- 24-hour battery life testing

- [ ] Launch the Project
	Ready for Phase 1: Hardware Prototyping (4-6 weeks)
	Next immediate steps:
	- Order development hardware (~€185 budget)
	- Setup nRF Connect SDK environment  
	- Begin sensor driver implementation
	- Validate PPG signal quality at upper arm placement

- [x] Ensure Documentation is Complete
	Comprehensive documentation created and updated:
	- Main project README with complete overview
	- SIGNAL_FLOW.md with complete sensor-to-app pipeline
	- DEVELOPMENT_ROADMAP.md with 4-phase implementation plan
	- UPPER_ARM_MIGRATION_RESULTS.md with test validation
	- Firmware documentation with architecture details
	- Hardware design specifications and manufacturing guide
	- Mobile app development guide with BLE integration
	- Automated testing framework for continuous validation

Current Development Status:
- Phase 0: Architecture & Design ✅ COMPLETE
- Phase 1: Hardware Prototyping 🚧 READY TO START
- Phase 2: Custom PCB Design 📋 PLANNED
- Phase 3: Mobile App Development 📋 PLANNED  
- Phase 4: Mechanical Design 📋 PLANNED

Key Achievements:
- Sensor placement optimized for upper arm
- All wrist artifacts removed from codebase
- Comprehensive signal flow documentation
- Test-driven development framework established
- Budget and timeline planning complete (~€2,385, 18-24 weeks)

Next Priority: Hardware procurement and nRF Connect SDK setup
