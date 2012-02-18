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
 * $brcm_Workfile: bchp_sun_rg.h $
 * $brcm_Revision: Hydra_Software_Devel/1 $
 * $brcm_Date: 7/17/09 8:20p $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Fri Jul 17 19:43:19 2009
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
 * $brcm_Log: /magnum/basemodules/chp/70015/rdb/a0/bchp_sun_rg.h $
 * 
 * Hydra_Software_Devel/1   7/17/09 8:20p albertl
 * PR56880: Initial revision.
 *
 ***************************************************************************/

#ifndef BCHP_SUN_RG_H__
#define BCHP_SUN_RG_H__

/***************************************************************************
 *SUN_RG - Registers for the Sundry block's RG  bridge
 ***************************************************************************/
#define BCHP_SUN_RG_REVISION                     0x00401000 /* RG Bridge Revision */
#define BCHP_SUN_RG_CTRL                         0x00401004 /* RG Bridge Control Register */
#define BCHP_SUN_RG_SW_RESET_0                   0x00401008 /* RG Bridge Software Reset 0 Register */
#define BCHP_SUN_RG_SW_RESET_1                   0x0040100c /* RG Bridge Software Reset 1 Register */

/***************************************************************************
 *REVISION - RG Bridge Revision
 ***************************************************************************/
/* SUN_RG :: REVISION :: reserved0 [31:16] */
#define BCHP_SUN_RG_REVISION_reserved0_MASK                        0xffff0000
#define BCHP_SUN_RG_REVISION_reserved0_SHIFT                       16

/* SUN_RG :: REVISION :: MAJOR [15:08] */
#define BCHP_SUN_RG_REVISION_MAJOR_MASK                            0x0000ff00
#define BCHP_SUN_RG_REVISION_MAJOR_SHIFT                           8

/* SUN_RG :: REVISION :: MINOR [07:00] */
#define BCHP_SUN_RG_REVISION_MINOR_MASK                            0x000000ff
#define BCHP_SUN_RG_REVISION_MINOR_SHIFT                           0

/***************************************************************************
 *CTRL - RG Bridge Control Register
 ***************************************************************************/
/* SUN_RG :: CTRL :: reserved0 [31:02] */
#define BCHP_SUN_RG_CTRL_reserved0_MASK                            0xfffffffc
#define BCHP_SUN_RG_CTRL_reserved0_SHIFT                           2

/* SUN_RG :: CTRL :: rbus_error_intr [01:01] */
#define BCHP_SUN_RG_CTRL_rbus_error_intr_MASK                      0x00000002
#define BCHP_SUN_RG_CTRL_rbus_error_intr_SHIFT                     1
#define BCHP_SUN_RG_CTRL_rbus_error_intr_INTR_DISABLE              0
#define BCHP_SUN_RG_CTRL_rbus_error_intr_INTR_ENABLE               1

/* SUN_RG :: CTRL :: reserved1 [00:00] */
#define BCHP_SUN_RG_CTRL_reserved1_MASK                            0x00000001
#define BCHP_SUN_RG_CTRL_reserved1_SHIFT                           0

/***************************************************************************
 *SW_RESET_0 - RG Bridge Software Reset 0 Register
 ***************************************************************************/
/* SUN_RG :: SW_RESET_0 :: reserved0 [31:01] */
#define BCHP_SUN_RG_SW_RESET_0_reserved0_MASK                      0xfffffffe
#define BCHP_SUN_RG_SW_RESET_0_reserved0_SHIFT                     1

/* SUN_RG :: SW_RESET_0 :: SPARE_SW_RESET [00:00] */
#define BCHP_SUN_RG_SW_RESET_0_SPARE_SW_RESET_MASK                 0x00000001
#define BCHP_SUN_RG_SW_RESET_0_SPARE_SW_RESET_SHIFT                0
#define BCHP_SUN_RG_SW_RESET_0_SPARE_SW_RESET_DEASSERT             0
#define BCHP_SUN_RG_SW_RESET_0_SPARE_SW_RESET_ASSERT               1

/***************************************************************************
 *SW_RESET_1 - RG Bridge Software Reset 1 Register
 ***************************************************************************/
/* SUN_RG :: SW_RESET_1 :: reserved0 [31:01] */
#define BCHP_SUN_RG_SW_RESET_1_reserved0_MASK                      0xfffffffe
#define BCHP_SUN_RG_SW_RESET_1_reserved0_SHIFT                     1

/* SUN_RG :: SW_RESET_1 :: SPARE_SW_RESET [00:00] */
#define BCHP_SUN_RG_SW_RESET_1_SPARE_SW_RESET_MASK                 0x00000001
#define BCHP_SUN_RG_SW_RESET_1_SPARE_SW_RESET_SHIFT                0
#define BCHP_SUN_RG_SW_RESET_1_SPARE_SW_RESET_DEASSERT             0
#define BCHP_SUN_RG_SW_RESET_1_SPARE_SW_RESET_ASSERT               1

#endif /* #ifndef BCHP_SUN_RG_H__ */

/* End of File */