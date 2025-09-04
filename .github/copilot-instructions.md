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

- [ ] Install Required Extensions
	No specific extensions required - project uses standard development tools

- [ ] Compile the Project
	Ready for compilation once development environment is set up:
	- Firmware: Requires nRF Connect SDK (Zephyr)
	- Mobile App: Requires Flutter SDK
	- Hardware: Requires KiCad or similar PCB design tools

- [ ] Create and Run Task
	Development tasks ready:
	- Firmware build and flash tasks
	- Mobile app build and test tasks
	- Hardware simulation and validation

- [ ] Launch the Project
	Project ready for development launch

- [x] Ensure Documentation is Complete
	Comprehensive documentation created:
	- Main project README with complete overview
	- Firmware documentation with architecture details
	- Hardware design specifications and manufacturing guide
	- Mobile app development guide with BLE integration
	- Complete documentation structure for all project phases
