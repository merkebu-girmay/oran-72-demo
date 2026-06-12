/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef _ORAN_ISOLATE_H_
#define _ORAN_ISOLATE_H_

#include <stdio.h>

#include <pthread.h>
#include <stdint.h>

#include "xran_fh_o_du.h"
#include "openair1/PHY/impl_defs_nr.h"
#include "openair1/PHY/TOOLS/tools_defs.h"
#include "openair1/PHY/defs_nr_common.h"
#include "openair1/PHY/NR_TRANSPORT/nr_transport_proto.h"
#include "openair1/PHY/defs_RU.h"

void print_fhi_counters(RU_t *ru, const int frame, const int slot);

void fh_on_timing_sync(RU_t *ru,
                               uint64_t gps_second,
                               uint32_t tti_counter,
                               int frame,
                               int slot);

// fh_south_out for split 7.2
void oran_tx_slot(RU_t *ru, int frame, int slot, uint64_t timestamp);
/** @brief Writes CP UL data for given slot. */
int xran_send_cp_ul_slot(RU_t *ru, int frame, int slot);
/** @brief Writes TX data (PDSCH) of given slot. */
int xran_fh_tx_send_slot(RU_t *ru, int frame, int slot, uint64_t timestamp);

#endif /* _ORAN_ISOLATE_H_ */
