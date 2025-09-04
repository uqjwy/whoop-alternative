# Contributing to Whoop Alternative

Thank you for your interest in contributing to the Whoop Alternative project! This document provides guidelines and information for contributors.

## Code of Conduct

We are committed to providing a welcoming and inclusive environment for all contributors. Please be respectful and constructive in all interactions.

## How to Contribute

### Reporting Issues

Before creating an issue, please:
1. Search existing issues to avoid duplicates
2. Use the appropriate issue template
3. Provide detailed information including:
   - Hardware/software versions
   - Steps to reproduce
   - Expected vs actual behavior
   - Screenshots or logs when relevant

### Development Workflow

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/yourusername/whoop-alternative.git
   cd whoop-alternative
   ```
3. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```
4. **Make your changes** following the coding standards
5. **Test your changes** thoroughly
6. **Commit with descriptive messages**:
   ```bash
   git commit -m "feat: add heart rate variability calculation"
   ```
7. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```
8. **Create a Pull Request** with a clear description

### Commit Message Convention

We use conventional commits for clear project history:

- `feat:` - New features
- `fix:` - Bug fixes  
- `docs:` - Documentation changes
- `style:` - Code style changes (formatting, no logic changes)
- `refactor:` - Code refactoring
- `test:` - Adding or updating tests
- `chore:` - Build process, tool changes, dependencies

Example: `feat(firmware): implement PPG signal filtering`

## Development Setup

### Prerequisites

#### Firmware Development
```bash
# Install nRF Connect SDK
west init -m https://github.com/nrfconnect/sdk-nrf --mr main
west update

# Install Python dependencies
pip install west pyocd

# Hardware requirements
# - nRF52840 Development Kit
# - MAX86141 Evaluation Kit or breakout board
# - BMA400 breakout board
```

#### Mobile App Development
```bash
# Install Flutter
flutter doctor

# Get dependencies
cd app/mobile
flutter pub get

# Run tests
flutter test
```

#### Documentation
```bash
# Install MkDocs
pip install mkdocs mkdocs-material

# Serve locally
mkdocs serve
```

### Building the Project

#### Firmware
```bash
cd firmware/app
west build -b nrf52840dk_nrf52840
west flash  # Flash to connected device
```

#### Mobile App
```bash
cd app/mobile
flutter run  # Run on connected device/simulator
flutter build apk  # Build Android APK
```

## Coding Standards

### C/C++ (Firmware)

- Use **4 spaces** for indentation
- Follow **Zephyr coding style**
- Maximum line length: **100 characters**
- Use descriptive variable and function names
- Comment complex algorithms and hardware interactions
- Include Doxygen documentation for public APIs

Example:
```c
/**
 * @brief Calculate heart rate from RR intervals
 * 
 * @param rr_intervals Array of RR intervals in milliseconds
 * @param count Number of intervals in the array
 * @return Heart rate in beats per minute, or 0 if invalid
 */
uint16_t calculate_heart_rate(uint32_t *rr_intervals, uint8_t count)
{
    if (rr_intervals == NULL || count < MIN_RR_INTERVALS) {
        return 0;
    }
    
    // Calculate average RR interval
    uint32_t sum = 0;
    for (uint8_t i = 0; i < count; i++) {
        sum += rr_intervals[i];
    }
    
    uint32_t avg_rr_ms = sum / count;
    return (60000 / avg_rr_ms);  // Convert to BPM
}
```

### Dart (Mobile App)

- Use **2 spaces** for indentation
- Follow **Dart style guide**
- Use `dart format` before committing
- Prefer `const` constructors where possible
- Use meaningful widget and class names
- Write widget tests for UI components

Example:
```dart
class HeartRateChart extends StatelessWidget {
  const HeartRateChart({
    Key? key,
    required this.heartRateData,
    this.showGrid = true,
  }) : super(key: key);

  final List<HeartRatePoint> heartRateData;
  final bool showGrid;

  @override
  Widget build(BuildContext context) {
    return LineChart(
      LineChartData(
        gridData: FlGridData(show: showGrid),
        titlesData: _buildTitlesData(),
        lineBarsData: _buildLineData(),
      ),
    );
  }
  
  // ... rest of implementation
}
```

### Documentation

- Use **Markdown** for all documentation
- Include code examples where helpful
- Keep language clear and concise
- Update relevant docs when changing functionality
- Use diagrams for complex system interactions

## Testing Requirements

### Firmware Testing

- **Unit tests** for signal processing algorithms
- **Hardware-in-the-loop tests** for sensor drivers
- **Integration tests** for BLE communication
- **Power consumption validation**

### Mobile App Testing

- **Widget tests** for UI components
- **Unit tests** for business logic
- **Integration tests** for BLE communication
- **Performance tests** for chart rendering

### Test Coverage

- Aim for **>80% code coverage** for critical modules
- All new features must include tests
- Bug fixes must include regression tests

## Hardware Contributions

### PCB Design

- Use **KiCad 6.0+** for schematic and PCB design
- Follow **IPC standards** for PCB layout
- Include **design rule checks** (DRC) results
- Provide **assembly drawings** and **bill of materials**
- Document **design decisions** and **component selections**

### Mechanical Design

- Use **FreeCAD** or **Fusion 360** for 3D modeling
- Provide **STEP files** for mechanical parts
- Include **assembly instructions**
- Consider **manufacturing constraints**
- Test **3D printed prototypes** before submission

## Documentation Contributions

- Follow existing documentation structure
- Use proper Markdown formatting
- Include relevant diagrams and code examples
- Test all code snippets
- Update table of contents and cross-references

## Review Process

### Pull Request Requirements

- [ ] Clear description of changes
- [ ] Tests pass locally
- [ ] Code follows style guidelines
- [ ] Documentation updated if needed
- [ ] No merge conflicts
- [ ] Linked to relevant issues

### Review Criteria

1. **Functionality** - Does it work as intended?
2. **Code Quality** - Is it readable and maintainable?
3. **Testing** - Are there adequate tests?
4. **Documentation** - Is it properly documented?
5. **Performance** - Does it meet performance requirements?
6. **Security** - Are there any security implications?

## Community

### Getting Help

- **GitHub Discussions** - For questions and general discussion
- **Issues** - For bug reports and feature requests
- **Discord/Slack** - Real-time chat (link TBD)

### Recognition

Contributors will be recognized in:
- README.md contributor list
- Release notes for significant contributions
- Annual contributor highlights

## Project Priorities

### Current Focus Areas

1. **Signal Processing** - PPG filtering and heart rate detection
2. **Power Optimization** - Battery life improvements
3. **BLE Communication** - Reliable data synchronization
4. **Mobile App UX** - Intuitive user interface
5. **Clinical Validation** - Accuracy testing and validation

### Future Roadmap

- ECG integration for improved accuracy
- Advanced sleep stage detection
- Machine learning for personalized insights
- Integration with health platforms
- International regulatory compliance

## Legal Considerations

### Intellectual Property

- All contributions become part of the open-source project
- Contributors retain copyright but grant project license
- Third-party code must be properly attributed
- No proprietary or confidential information

### Medical Disclaimer

- This is NOT a medical device
- Contributions must not make medical claims
- Include appropriate disclaimers
- Focus on fitness and wellness applications

---

Thank you for contributing to making health monitoring more accessible and open!
