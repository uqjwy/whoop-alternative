#!/bin/bash

# Test script to check for wrist-related artifacts in the codebase
# This script verifies that all references have been updated for upper arm placement

echo "=== Testing Codebase for Wrist-Related Artifacts ==="
echo "Checking for outdated wrist/handgelenk references..."

# Counter for issues found
ISSUES_FOUND=0

# Function to report an issue
report_issue() {
    echo "❌ ISSUE: $1"
    ISSUES_FOUND=$((ISSUES_FOUND + 1))
}

# Function to report success
report_success() {
    echo "✅ PASS: $1"
}

echo
echo "1. Checking for German wrist references..."
WRIST_GERMAN=$(grep -r "handgelenk" --include="*.md" --include="*.c" --include="*.h" --include="*.dart" . 2>/dev/null | wc -l)
if [ $WRIST_GERMAN -gt 0 ]; then
    echo "Found references:"
    grep -r "handgelenk" --include="*.md" --include="*.c" --include="*.h" --include="*.dart" . 2>/dev/null
    report_issue "Found German 'handgelenk' references that should be updated to 'oberarm'"
else
    report_success "No German 'handgelenk' references found"
fi

echo
echo "2. Checking for English wrist references..."
WRIST_ENGLISH=$(grep -r -i "wrist" --include="*.md" --include="*.c" --include="*.h" --include="*.dart" . 2>/dev/null | grep -v "build/" | grep -v ".dart_tool/" | grep -v ".pub-cache/" | wc -l)
if [ $WRIST_ENGLISH -gt 0 ]; then
    echo "Found references:"
    grep -r -i "wrist" --include="*.md" --include="*.c" --include="*.h" --include="*.dart" . 2>/dev/null | grep -v "build/" | grep -v ".dart_tool/" | grep -v ".pub-cache/"
    report_issue "Found English 'wrist' references that should be updated to 'upper arm' or 'bicep'"
else
    report_success "No English 'wrist' references found"
fi

echo
echo "3. Checking for proper upper arm references..."
UPPER_ARM_REFS=$(grep -r -i "oberarm\|upper.*arm\|bicep" --include="*.md" --include="*.c" --include="*.h" . 2>/dev/null | wc -l)
if [ $UPPER_ARM_REFS -gt 0 ]; then
    report_success "Found $UPPER_ARM_REFS proper upper arm references"
    echo "Sample references:"
    grep -r -i "oberarm\|upper.*arm\|bicep" --include="*.md" --include="*.c" --include="*.h" . 2>/dev/null | head -3
else
    report_issue "No upper arm references found - documentation might be incomplete"
fi

echo
echo "4. Checking motion artifact configurations..."
# Check if motion artifact thresholds are reasonable for upper arm placement
MOTION_CONFIGS=$(grep -r "motion.*threshold\|artifact.*threshold" --include="*.h" --include="*.c" --include="*.txt" . 2>/dev/null)
if [ -n "$MOTION_CONFIGS" ]; then
    report_success "Motion artifact configurations found and should be reviewed for upper arm placement"
    echo "Current configurations:"
    echo "$MOTION_CONFIGS"
    echo
    echo "NOTE: Upper arm placement typically requires:"
    echo "  - Lower motion thresholds (less movement than wrist)"
    echo "  - Adjusted filter cutoffs"
    echo "  - Different artifact detection patterns"
else
    report_issue "No motion artifact configurations found"
fi

echo
echo "5. Checking band/mechanical references..."
BAND_REFS=$(grep -r "band\|armband" --include="*.md" . 2>/dev/null)
if [ -n "$BAND_REFS" ]; then
    report_success "Band references found"
    echo "Current band references:"
    echo "$BAND_REFS" | head -3
else
    report_issue "No band references found"
fi

echo
echo "6. Testing firmware architecture for sensor placement agnostic design..."
if [ -f "firmware/test_architecture.sh" ]; then
    cd firmware
    if ./test_architecture.sh > /dev/null 2>&1; then
        report_success "Firmware architecture test passed - sensor placement agnostic"
    else
        report_issue "Firmware architecture test failed"
    fi
    cd ..
else
    report_issue "Firmware architecture test script not found"
fi

echo
echo "7. Testing mobile app for placement-specific references..."
if [ -d "app/mobile" ]; then
    cd app/mobile
    if flutter test > /dev/null 2>&1; then
        report_success "Mobile app tests passed"
    else
        report_issue "Mobile app tests failed"
    fi
    cd ../..
else
    report_issue "Mobile app directory not found"
fi

echo
echo "8. Checking documentation consistency..."
DOCS_WITH_PLACEMENT=$(grep -r -l "placement\|position.*sensor\|erfassung" docs/ 2>/dev/null | wc -l)
if [ $DOCS_WITH_PLACEMENT -gt 0 ]; then
    report_success "Found $DOCS_WITH_PLACEMENT documentation files mentioning sensor placement"
    echo "Files to review:"
    grep -r -l "placement\|position.*sensor\|erfassung" docs/ 2>/dev/null
else
    echo "ℹ️  INFO: No specific placement documentation found in docs/ directory"
fi

echo
echo "=== SUMMARY ==="
if [ $ISSUES_FOUND -eq 0 ]; then
    echo "🎉 SUCCESS: No wrist-related artifacts found!"
    echo "✅ All references have been properly updated for upper arm placement"
    echo "✅ Firmware architecture is sensor placement agnostic"
    echo "✅ Mobile app tests pass"
    echo
    echo "RECOMMENDATIONS for upper arm placement:"
    echo "• Review motion artifact thresholds (typically need to be lower)"
    echo "• Adjust PPG signal processing for different tissue characteristics"
    echo "• Update user documentation about proper device positioning"
    echo "• Consider different strap/band design for upper arm comfort"
else
    echo "⚠️  FOUND $ISSUES_FOUND ISSUES that need attention"
    echo "Please review and update the flagged items above"
fi

exit $ISSUES_FOUND
