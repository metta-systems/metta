//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
kconsole << "RIGHTS READ " << stretch_v1_rights(stretch_v1_right_read) << endl
         << "RIGHTS WRITE " << stretch_v1_rights(stretch_v1_right_write) << endl
         << "RIGHTS R/W " << stretch_v1_rights(stretch_v1_right_read).add(stretch_v1_right_write) << endl
         << "RIGHTS W/R " << stretch_v1_rights(stretch_v1_right_write).add(stretch_v1_right_read) << endl;
