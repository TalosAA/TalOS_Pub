#ifndef _CPU_VENDOR_H_
#define _CPU_VENDOR_H_

#define CPU_VENDOR_STR_LEN      (13)

/**
 * Vendor strings from CPUs.
 */
#define CPU_VENDOR_AMD           "AuthenticAMD"
#define CPU_VENDOR_AMD_OLD       "AMDisbetter!"
#define CPU_VENDOR_INTEL         "GenuineIntel"
#define CPU_VENDOR_VIA           "VIA VIA VIA "
#define CPU_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPU_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPU_VENDOR_CYRIX         "CyrixInstead"
#define CPU_VENDOR_CENTAUR       "CentaurHauls"
#define CPU_VENDOR_NEXGEN        "NexGenDriven"
#define CPU_VENDOR_UMC           "UMC UMC UMC "
#define CPU_VENDOR_SIS           "SiS SiS SiS "
#define CPU_VENDOR_NSC           "Geode by NSC"
#define CPU_VENDOR_RISE          "RiseRiseRise"
#define CPU_VENDOR_VORTEX        "Vortex86 SoC"
#define CPU_VENDOR_AO486         "MiSTer AO486"
#define CPU_VENDOR_AO486_OLD     "GenuineAO486"
#define CPU_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPU_VENDOR_HYGON         "HygonGenuine"
#define CPU_VENDOR_ELBRUS        "E2K MACHINE "

/**
 * Vendor strings from hypervisors/emulators.
 */
#define CPU_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPU_VENDOR_KVM           " KVMKVMKVM  "
#define CPU_VENDOR_VMWARE        "VMwareVMware"
#define CPU_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPU_VENDOR_XEN           "XenVMMXenVMM"
#define CPU_VENDOR_HYPERV        "Microsoft Hv"
#define CPU_VENDOR_PARALLELS     " prl hyperv "
#define CPU_VENDOR_PARALLELS_ALT " lrpepyh vr "
#define CPU_VENDOR_BHYVE         "bhyve bhyve "
#define CPU_VENDOR_QNX           " QNXQVMBSQG "

#endif
