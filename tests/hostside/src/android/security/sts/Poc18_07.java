/**
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.security.sts;

import static org.junit.Assert.*;
import android.platform.test.annotations.RootPermissionTest;
import org.junit.Test;
import org.junit.runner.RunWith;

public class Poc18_07 extends SecurityTestCase {

    /**
     * b/77238656
     */
    @RootPermissionTest
    public void testPocBug_77238656() throws Exception {
        final String PROPERTY_GET = "getprop libc.debug.malloc.options";
        final String PROPERTY_SET = "setprop libc.debug.malloc.options ";
        enableAdbRoot(getDevice());
        String oldProperty = AdbUtils.runCommandLine(PROPERTY_GET, getDevice());
        AdbUtils.runCommandLine(PROPERTY_SET + "fill", getDevice());
        AdbUtils.runCommandLine("logcat -c", getDevice());
        boolean retCode = AdbUtils.runPocCheckExitCode("Bug-77238656", getDevice(), 30);
        String logcat = AdbUtils.runCommandLine("logcat -d", getDevice());
        if ((oldProperty == null) || (oldProperty.isEmpty())) {
            AdbUtils.runCommandLine(PROPERTY_SET + "\"\"", getDevice());
        } else {
            AdbUtils.runCommandLine(PROPERTY_SET + "\"" + oldProperty + "\"", getDevice());
        }
        assertFalse("Metadata padding is not empty", retCode);
    }
}
