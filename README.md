# Android Security Test Suite (STS)

Security Test Suite Trade Federation, sts-tradefed for short is build on op of the Android Trade Federation test harness to test all android devices for security patch tests that do not fall into the Compatibility Test Suite.  These tests are exclusively for fixes that are associated (or will be associated) with a CVE.


To run a test plan on a single device:

1. Make sure you have at least one device connected

2. Launch the sts-tradefed console by running the 'sts-tradefed'. If you've downloaded and extracted the STS zip, the script can be found at
  android-sts/tools/sts-tradefed
Or else if you are working from the Android source tree and have run "make sts", the script can be found at out/host/linux-x86/sts/android-sts/tools/sts-tradefed
3. Type:
 'run sts' to run the default sTS plan
