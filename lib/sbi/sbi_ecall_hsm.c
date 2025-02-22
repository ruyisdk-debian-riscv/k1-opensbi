/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Atish Patra <atish.patra@wdc.com>
 */

#include <sbi/sbi_domain.h>
#include <sbi/sbi_ecall.h>
#include <sbi/sbi_ecall_interface.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_hsm.h>
#include <sbi/sbi_scratch.h>
#include <sbi/riscv_asm.h>

static int sbi_ecall_hsm_handler(unsigned long extid, unsigned long funcid,
				 struct sbi_trap_regs *regs,
				 struct sbi_ecall_return *out)
{
	int ret = 0;
	struct sbi_scratch *scratch = sbi_scratch_thishart_ptr();
	ulong smode = (csr_read(CSR_MSTATUS) & MSTATUS_MPP) >>
			MSTATUS_MPP_SHIFT;

	switch (funcid) {
	case SBI_EXT_HSM_HART_START:
		ret = sbi_hsm_hart_start(scratch, sbi_domain_thishart_ptr(),
					 regs->a0, regs->a1, smode, regs->a2);
		break;
	case SBI_EXT_HSM_HART_STOP:
		ret = sbi_hsm_hart_stop(scratch, true);
		break;
	case SBI_EXT_HSM_HART_GET_STATUS:
#ifndef CONFIG_ARM_PSCI_SUPPORT
		ret = sbi_hsm_hart_get_state(sbi_domain_thishart_ptr(),
					     regs->a0);
#else
		ret = sbi_hsm_hart_get_psci_state(sbi_domain_thishart_ptr(),
					     regs->a0);
#endif
		break;
	case SBI_EXT_HSM_HART_SUSPEND:
		ret = sbi_hsm_hart_suspend(scratch, regs->a0, regs->a1,
					   smode, regs->a2);
		break;
	default:
		ret = SBI_ENOTSUPP;
	}

	if (ret >= 0) {
		out->value = ret;
		ret = 0;
	}

	return ret;
}

struct sbi_ecall_extension ecall_hsm;

static int sbi_ecall_hsm_register_extensions(void)
{
	return sbi_ecall_register_extension(&ecall_hsm);
}

struct sbi_ecall_extension ecall_hsm = {
	.extid_start		= SBI_EXT_HSM,
	.extid_end		= SBI_EXT_HSM,
	.register_extensions	= sbi_ecall_hsm_register_extensions,
	.handle			= sbi_ecall_hsm_handler,
};
