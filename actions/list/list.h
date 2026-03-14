#pragma once

int list(); /* List all processes, return -1 for error, 0 for success */

/* List process by ID, return -1 for error, 0 for success */
int list_by_id(const long id);
