//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
kconsole << "RIGHTS READ " << stretch_v1_rights(stretch_v1_right_read) << endl
         << "RIGHTS WRITE " << stretch_v1_rights(stretch_v1_right_write) << endl
         << "RIGHTS R/W " << stretch_v1_rights(stretch_v1_right_read).add(stretch_v1_right_write) << endl
         << "RIGHTS W/R " << stretch_v1_rights(stretch_v1_right_write).add(stretch_v1_right_read) << endl;
