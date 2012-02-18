/***************************************************************************
 *     Copyright (c) 1999-2009, Broadcom Corporation
 *
 **********************************************************************
 * This file is part of the crystalhd device driver.
 *
 * This driver is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this driver.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************
 *
 * $brcm_Workfile: bchp_xpt_pb1.h $
 * $brcm_Revision: Hydra_Software_Devel/1 $
 * $brcm_Date: 7/17/09 8:24p $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Fri Jul 17 19:42:51 2009
 *                 MD5 Checksum         2914699efc3fb3edefca5cb4f4f38b34
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: /magnum/basemodules/chp/70015/rdb/a0/bchp_xpt_pb1.h $
 * 
 * Hydra_Software_Devel/1   7/17/09 8:24p albertl
 * PR56880: Initial revision.
 *
 ***************************************************************************/

#ifndef BCHP_XPT_PB1_H__
#define BCHP_XPT_PB1_H__

/***************************************************************************
 *XPT_PB1 - Playback 1 Control Registers
 ***************************************************************************/
#define BCHP_XPT_PB1_CTRL1                       0x0020b080 /* Playback Control 1 Register */
#define BCHP_XPT_PB1_CTRL2                       0x0020b084 /* Playback Control 2 Register */
#define BCHP_XPT_PB1_CTRL3                       0x0020b088 /* Playback Control 3 Register */
#define BCHP_XPT_PB1_CTRL4                       0x0020b08c /* Playback Control 4 Register */
#define BCHP_XPT_PB1_FIRST_DESC_ADDR             0x0020b090 /* Playback First Descriptor Address Register */
#define BCHP_XPT_PB1_CURR_DESC_ADDR              0x0020b094 /* Playback Current Descriptor Address Register */
#define BCHP_XPT_PB1_CURR_BUFF_ADDR              0x0020b098 /* Playback Current Buffer Address Register */
#define BCHP_XPT_PB1_BLOCKOUT                    0x0020b09c /* Data Transport Playback Block Out Control */
#define BCHP_XPT_PB1_PKTZ_CONTEXT0               0x0020b0a0 /* Data Transport Playback Packetize Mode Context 0 Control */
#define BCHP_XPT_PB1_PKTZ_CONTEXT1               0x0020b0a4 /* Data Transport Playback Packetize Mode Context 1 Control */
#define BCHP_XPT_PB1_PKTZ_CONTEXT2               0x0020b0a8 /* Data Transport Playback Packetize Mode Context 2 Control */
#define BCHP_XPT_PB1_PKTZ_CONTEXT3               0x0020b0ac /* Data Transport Playback Packetize Mode Context 3 Control */
#define BCHP_XPT_PB1_TS_ERR_BOUND                0x0020b0b0 /* Data Transport Playback Timestamp Error Bound Register */
#define BCHP_XPT_PB1_PARSER_CTRL1                0x0020b0b4 /* Data Transport Playback Parser Control Register */
#define BCHP_XPT_PB1_PARSER_CTRL2                0x0020b0b8 /* Data Transport Playback Parser Control Register 2 */
#define BCHP_XPT_PB1_PARSER_TIMESTAMP            0x0020b0bc /* Data Transport Playback Parser Local Timestamp */
#define BCHP_XPT_PB1_INTR                        0x0020b0c0 /* Playback Processing Error and Status Interrupt Register */
#define BCHP_XPT_PB1_INTR_EN                     0x0020b0c4 /* Playback Processing Error and Status Interrupt Enable Register */
#define BCHP_XPT_PB1_INTR_TAGS                   0x0020b0c8 /* Playback Interrupt Tag Register */

#endif /* #ifndef BCHP_XPT_PB1_H__ */

/* End of File */