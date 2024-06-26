/*
 * MicroEmulator
 * Copyright (C) 2006-2007 Bartek Teodorczyk <barteo@barteo.net>
 * Copyright (C) 2006-2007 Vlad Skarzhevskyy
 * <p>
 * It is licensed under the following two licenses as alternatives:
 * 1. GNU Lesser General Public License (the "LGPL") version 2.1 or any newer version
 * 2. Apache License (the "AL") Version 2.0
 * <p>
 * You may not use this file except in compliance with at least one of
 * the above two licenses.
 * <p>
 * You may obtain a copy of the LGPL at
 * http://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
 * <p>
 * You may obtain a copy of the AL at
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the LGPL or the AL for the specific language governing permissions and
 * limitations.
 *
 * @version $Id$
 */
package org.microemu.cldc.file;

import org.microemu.microedition.io.ConnectionImplementation;

import java.io.IOException;
import java.net.URISyntaxException;

/**
 * This is default Connection when no initialization has been made.
 *
 * @author vlads
 */
public class Connection implements ConnectionImplementation {

	public final static String PROTOCOL = "file://";

	public static final int CONNECTIONTYPE_SYSTEM_FS = 0;

	private static int connectionType = CONNECTIONTYPE_SYSTEM_FS;

	@Override
	public javax.microedition.io.Connection openConnection(String name, int mode, boolean timeouts) throws IOException {
		// file://<host>/<path>
		if (!name.startsWith(PROTOCOL)) {
			throw new IOException("Invalid Protocol " + name);
		}
		if (connectionType == CONNECTIONTYPE_SYSTEM_FS) {
			return new FileSystemFileConnection(name);
		}
		throw new IOException("Invalid connectionType configuration");
	}

	static int getConnectionType() {
		return connectionType;
	}

	static void setConnectionType(int connectionType) {
		Connection.connectionType = connectionType;
	}

}
