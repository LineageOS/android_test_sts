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

import android.platform.test.annotations.RootPermissionTest;
import com.android.tradefed.testtype.DeviceJUnit4ClassRunner;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(DeviceJUnit4ClassRunner.class)
public class Poc18_06 extends SecurityTestCase {

    /**
     * b/77486542
     */
    @RootPermissionTest
    @Test
    public void testPocBug_77486542() throws Exception {
        enableAdbRoot(getDevice());
        String output = AdbUtils.runPoc("Bug-77486542", getDevice(), 30);
        assertNotMatches("[\\s\\n\\S]*77486542[\\s\\n\\S]*", output);
    }
}
