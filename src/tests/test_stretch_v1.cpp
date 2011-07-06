kconsole << "RIGHTS READ " << stretch_v1_rights(stretch_v1_right_read) << endl
         << "RIGHTS WRITE " << stretch_v1_rights(stretch_v1_right_write) << endl
         << "RIGHTS R/W " << stretch_v1_rights(stretch_v1_right_read).add(stretch_v1_right_write) << endl
         << "RIGHTS W/R " << stretch_v1_rights(stretch_v1_right_write).add(stretch_v1_right_read) << endl;
