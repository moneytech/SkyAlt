/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2024-11-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

StdProgress StdProgress_init(void)
{
	StdProgress self;
	self.title = 0;
	self.done = 0;
	self.phase = 0;
	self.num_phases = 1;
	self.running = TRUE;
	return self;
}

void StdProgress_free(StdProgress* self)
{
	Os_memset(self, sizeof(StdProgress));
}

void StdProgress_setNumPhases(volatile StdProgress* self, UINT num_phases)
{
	self->phase = -1;
	self->num_phases = num_phases;
	self->done = 0;
	self->running = TRUE;
}

void StdProgress_addNextPhase(volatile StdProgress* self, const UNI* title)
{
	self->phase++;
	self->done = 0;
	self->title = title;
}

int StdProgress_getPhase(volatile const StdProgress* self)
{
	return Std_max(1, self->phase);
}
void StdProgress_reset(volatile StdProgress* self)
{
	StdProgress_setNumPhases(self, 1);
}
