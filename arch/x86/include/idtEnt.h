/*
 * Copyright (c) 2012-2014, Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * @brief IA-32 IDT Entry code
 *
 * This header file provides code for constructing an IA-32 interrupt
 * descriptor.
 */

#ifndef _IDTENT_H
#define _IDTENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bitmask used to determine which exceptions result in an error code being
 * pushed onto the stack.  The following exception vectors push an error code:
 *
 *  Vector    Mnemonic    Description
 *  ------    -------     ----------------------
 *    8       #DF         Double Fault
 *    10      #TS         Invalid TSS
 *    11      #NP         Segment Not Present
 *    12      #SS         Stack Segment Fault
 *    13      #GP         General Protection Fault
 *    14      #PF         Page Fault
 *    17      #AC         Alignment Check
 */
#define _EXC_ERROR_CODE_FAULTS	0x27d00


/**
 * @brief Interrupt Descriptor Table (IDT) entry structure
 *
 * See section 6.11 in x86 CPU manual vol. 3A
 */
typedef struct idt_entry {
	union {
		uint16_t offset_low;
		uint16_t reserved_task_gate_0;
	};
	uint16_t segment_selector;
	union {
		struct {
			uint8_t reserved:5;
			uint8_t always_0_0:3;
		};
		uint8_t reserved_task_gate_1;
	};
	uint8_t type:3;		/* task:101, irq:110, trap:111 */
	uint8_t gate_size:1;	/* size of gate, 1: 32-bit, 0:16-bit */
	uint8_t always_0_1:1;
	uint8_t dpl:2;		/* Descriptor privilege level */
	uint8_t present:1;	/* present yes/no */
	union {
		uint16_t offset_high;
		uint16_t reserved_task_gate_2;
	};
} __packed IDT_ENTRY;

/**
 *
 * @brief Create an IDT entry
 *
 * This routine creates an interrupt-gate descriptor at the location defined by
 * @a pIdtEntry. The entry is created such that @a routine is invoked when an
 * interrupt vector is asserted.  The @a dpl argument specifies the privilege
 * level for the interrupt-gate descriptor; (hardware) interrupts and exceptions
 * should specify a level of 0, whereas handlers for user-mode software generated
 * interrupts should specify 3.
 *
 * @param pIdtEntry Pointer to where the entry is be built
 * @param routine Routine to call when interrupt occurs
 * @param dpl Private level for interrupt descriptor
 * @return N/A
 *
 * INTERNAL
 * This is a shared routine between the IA-32 nanokernel runtime code and the
 * genIdt host tool code. It is done this way to keep the two sides in sync.
 *
 * The runtime passes a pointer directly to the IDT entry to update whereas the
 * host side simply passes a pointer to a local variable.
 *
 */
static inline void _IdtEntCreate(uint64_t *pIdtEntry, uint32_t routine,
				 unsigned int dpl)
{
	uint32_t *pIdtEntry32 = (uint32_t *)pIdtEntry;

	pIdtEntry32[0] = (KERNEL_CODE_SEG_SELECTOR << 16) |
			((uint16_t)routine);

	/*
	 * The constant 0x8e00 results from the following:
	 *
	 * Segment Present = 1
	 *
	 * Descriptor Privilege Level (DPL) = 0  (dpl arg will be or'ed in)
	 *
	 * Interrupt Gate Indicator = 0xE
	 *    The _IntEnt() and _ExcEnt() stubs assume that an interrupt-gate
	 *    descriptor is used, and thus they do not issue a 'cli' instruction
	 *    given that the processor automatically clears the IF flag when
	 *    accessing the interrupt/exception handler via an interrupt-gate.
	 *
	 * Size of Gate (D) = 1
	 *
	 * Reserved = 0
	 */

	pIdtEntry32[1] = (routine & 0xffff0000) | (0x8e00 | (dpl << 13));
}

#ifdef __cplusplus
}
#endif

#endif /* _IDTENT_H */
