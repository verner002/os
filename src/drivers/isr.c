/**
 * @file isr.c
 * @author verner002
 * @date 28/09/2025
*/

#include "types.h"
#include "drivers/cpu.h"

extern void __isr(uint8_t vector, struct __interrupt_frame *frame);

__attribute__((interrupt)) static void __int0_handler(struct __interrupt_frame *frame) {
    __isr(0, frame);
}

__attribute__((interrupt)) static void __int1_handler(struct __interrupt_frame *frame) {
    __isr(1, frame);
}

__attribute__((interrupt)) static void __int2_handler(struct __interrupt_frame *frame) {
    __isr(2, frame);
}

__attribute__((interrupt)) static void __int3_handler(struct __interrupt_frame *frame) {
    __isr(3, frame);
}

__attribute__((interrupt)) static void __int4_handler(struct __interrupt_frame *frame) {
    __isr(4, frame);
}

__attribute__((interrupt)) static void __int5_handler(struct __interrupt_frame *frame) {
    __isr(5, frame);
}

__attribute__((interrupt)) static void __int6_handler(struct __interrupt_frame *frame) {
    __isr(6, frame);
}

__attribute__((interrupt)) static void __int7_handler(struct __interrupt_frame *frame) {
    __isr(7, frame);
}

__attribute__((interrupt)) static void __int8_handler(struct __interrupt_frame *frame) {
    __isr(8, frame);
}

__attribute__((interrupt)) static void __int9_handler(struct __interrupt_frame *frame) {
    __isr(9, frame);
}

__attribute__((interrupt)) static void __int10_handler(struct __interrupt_frame *frame) {
    __isr(10, frame);
}

__attribute__((interrupt)) static void __int11_handler(struct __interrupt_frame *frame) {
    __isr(11, frame);
}

__attribute__((interrupt)) static void __int12_handler(struct __interrupt_frame *frame) {
    __isr(12, frame);
}

__attribute__((interrupt)) static void __int13_handler(struct __interrupt_frame *frame) {
    __isr(13, frame);
}

__attribute__((interrupt)) static void __int14_handler(struct __interrupt_frame *frame) {
    __isr(14, frame);
}

__attribute__((interrupt)) static void __int15_handler(struct __interrupt_frame *frame) {
    __isr(15, frame);
}

__attribute__((interrupt)) static void __int16_handler(struct __interrupt_frame *frame) {
    __isr(16, frame);
}

__attribute__((interrupt)) static void __int17_handler(struct __interrupt_frame *frame) {
    __isr(17, frame);
}

__attribute__((interrupt)) static void __int18_handler(struct __interrupt_frame *frame) {
    __isr(18, frame);
}

__attribute__((interrupt)) static void __int19_handler(struct __interrupt_frame *frame) {
    __isr(19, frame);
}

__attribute__((interrupt)) static void __int20_handler(struct __interrupt_frame *frame) {
    __isr(20, frame);
}

__attribute__((interrupt)) static void __int21_handler(struct __interrupt_frame *frame) {
    __isr(21, frame);
}

__attribute__((interrupt)) static void __int22_handler(struct __interrupt_frame *frame) {
    __isr(22, frame);
}

__attribute__((interrupt)) static void __int23_handler(struct __interrupt_frame *frame) {
    __isr(23, frame);
}

__attribute__((interrupt)) static void __int24_handler(struct __interrupt_frame *frame) {
    __isr(24, frame);
}

__attribute__((interrupt)) static void __int25_handler(struct __interrupt_frame *frame) {
    __isr(25, frame);
}

__attribute__((interrupt)) static void __int26_handler(struct __interrupt_frame *frame) {
    __isr(26, frame);
}

__attribute__((interrupt)) static void __int27_handler(struct __interrupt_frame *frame) {
    __isr(27, frame);
}

__attribute__((interrupt)) static void __int28_handler(struct __interrupt_frame *frame) {
    __isr(28, frame);
}

__attribute__((interrupt)) static void __int29_handler(struct __interrupt_frame *frame) {
    __isr(29, frame);
}

__attribute__((interrupt)) static void __int30_handler(struct __interrupt_frame *frame) {
    __isr(30, frame);
}

__attribute__((interrupt)) static void __int31_handler(struct __interrupt_frame *frame) {
    __isr(31, frame);
}

__attribute__((interrupt)) static void __int32_handler(struct __interrupt_frame *frame) {
    __isr(32, frame);
}

__attribute__((interrupt)) static void __int33_handler(struct __interrupt_frame *frame) {
    __isr(33, frame);
}

__attribute__((interrupt)) static void __int34_handler(struct __interrupt_frame *frame) {
    __isr(34, frame);
}

__attribute__((interrupt)) static void __int35_handler(struct __interrupt_frame *frame) {
    __isr(35, frame);
}

__attribute__((interrupt)) static void __int36_handler(struct __interrupt_frame *frame) {
    __isr(36, frame);
}

__attribute__((interrupt)) static void __int37_handler(struct __interrupt_frame *frame) {
    __isr(37, frame);
}

__attribute__((interrupt)) static void __int38_handler(struct __interrupt_frame *frame) {
    __isr(38, frame);
}

__attribute__((interrupt)) static void __int39_handler(struct __interrupt_frame *frame) {
    __isr(39, frame);
}

__attribute__((interrupt)) static void __int40_handler(struct __interrupt_frame *frame) {
    __isr(40, frame);
}

__attribute__((interrupt)) static void __int41_handler(struct __interrupt_frame *frame) {
    __isr(41, frame);
}

__attribute__((interrupt)) static void __int42_handler(struct __interrupt_frame *frame) {
    __isr(42, frame);
}

__attribute__((interrupt)) static void __int43_handler(struct __interrupt_frame *frame) {
    __isr(43, frame);
}

__attribute__((interrupt)) static void __int44_handler(struct __interrupt_frame *frame) {
    __isr(44, frame);
}

__attribute__((interrupt)) static void __int45_handler(struct __interrupt_frame *frame) {
    __isr(45, frame);
}

__attribute__((interrupt)) static void __int46_handler(struct __interrupt_frame *frame) {
    __isr(46, frame);
}

__attribute__((interrupt)) static void __int47_handler(struct __interrupt_frame *frame) {
    __isr(47, frame);
}

__attribute__((interrupt)) static void __int48_handler(struct __interrupt_frame *frame) {
    __isr(48, frame);
}

__attribute__((interrupt)) static void __int49_handler(struct __interrupt_frame *frame) {
    __isr(49, frame);
}

__attribute__((interrupt)) static void __int50_handler(struct __interrupt_frame *frame) {
    __isr(50, frame);
}

__attribute__((interrupt)) static void __int51_handler(struct __interrupt_frame *frame) {
    __isr(51, frame);
}

__attribute__((interrupt)) static void __int52_handler(struct __interrupt_frame *frame) {
    __isr(52, frame);
}

__attribute__((interrupt)) static void __int53_handler(struct __interrupt_frame *frame) {
    __isr(53, frame);
}

__attribute__((interrupt)) static void __int54_handler(struct __interrupt_frame *frame) {
    __isr(54, frame);
}

__attribute__((interrupt)) static void __int55_handler(struct __interrupt_frame *frame) {
    __isr(55, frame);
}

__attribute__((interrupt)) static void __int56_handler(struct __interrupt_frame *frame) {
    __isr(56, frame);
}

__attribute__((interrupt)) static void __int57_handler(struct __interrupt_frame *frame) {
    __isr(57, frame);
}

__attribute__((interrupt)) static void __int58_handler(struct __interrupt_frame *frame) {
    __isr(58, frame);
}

__attribute__((interrupt)) static void __int59_handler(struct __interrupt_frame *frame) {
    __isr(59, frame);
}

__attribute__((interrupt)) static void __int60_handler(struct __interrupt_frame *frame) {
    __isr(60, frame);
}

__attribute__((interrupt)) static void __int61_handler(struct __interrupt_frame *frame) {
    __isr(61, frame);
}

__attribute__((interrupt)) static void __int62_handler(struct __interrupt_frame *frame) {
    __isr(62, frame);
}

__attribute__((interrupt)) static void __int63_handler(struct __interrupt_frame *frame) {
    __isr(63, frame);
}

__attribute__((interrupt)) static void __int64_handler(struct __interrupt_frame *frame) {
    __isr(64, frame);
}

__attribute__((interrupt)) static void __int65_handler(struct __interrupt_frame *frame) {
    __isr(65, frame);
}

__attribute__((interrupt)) static void __int66_handler(struct __interrupt_frame *frame) {
    __isr(66, frame);
}

__attribute__((interrupt)) static void __int67_handler(struct __interrupt_frame *frame) {
    __isr(67, frame);
}

__attribute__((interrupt)) static void __int68_handler(struct __interrupt_frame *frame) {
    __isr(68, frame);
}

__attribute__((interrupt)) static void __int69_handler(struct __interrupt_frame *frame) {
    __isr(69, frame);
}

__attribute__((interrupt)) static void __int70_handler(struct __interrupt_frame *frame) {
    __isr(70, frame);
}

__attribute__((interrupt)) static void __int71_handler(struct __interrupt_frame *frame) {
    __isr(71, frame);
}

__attribute__((interrupt)) static void __int72_handler(struct __interrupt_frame *frame) {
    __isr(72, frame);
}

__attribute__((interrupt)) static void __int73_handler(struct __interrupt_frame *frame) {
    __isr(73, frame);
}

__attribute__((interrupt)) static void __int74_handler(struct __interrupt_frame *frame) {
    __isr(74, frame);
}

__attribute__((interrupt)) static void __int75_handler(struct __interrupt_frame *frame) {
    __isr(75, frame);
}

__attribute__((interrupt)) static void __int76_handler(struct __interrupt_frame *frame) {
    __isr(76, frame);
}

__attribute__((interrupt)) static void __int77_handler(struct __interrupt_frame *frame) {
    __isr(77, frame);
}

__attribute__((interrupt)) static void __int78_handler(struct __interrupt_frame *frame) {
    __isr(78, frame);
}

__attribute__((interrupt)) static void __int79_handler(struct __interrupt_frame *frame) {
    __isr(79, frame);
}

__attribute__((interrupt)) static void __int80_handler(struct __interrupt_frame *frame) {
    __isr(80, frame);
}

__attribute__((interrupt)) static void __int81_handler(struct __interrupt_frame *frame) {
    __isr(81, frame);
}

__attribute__((interrupt)) static void __int82_handler(struct __interrupt_frame *frame) {
    __isr(82, frame);
}

__attribute__((interrupt)) static void __int83_handler(struct __interrupt_frame *frame) {
    __isr(83, frame);
}

__attribute__((interrupt)) static void __int84_handler(struct __interrupt_frame *frame) {
    __isr(84, frame);
}

__attribute__((interrupt)) static void __int85_handler(struct __interrupt_frame *frame) {
    __isr(85, frame);
}

__attribute__((interrupt)) static void __int86_handler(struct __interrupt_frame *frame) {
    __isr(86, frame);
}

__attribute__((interrupt)) static void __int87_handler(struct __interrupt_frame *frame) {
    __isr(87, frame);
}

__attribute__((interrupt)) static void __int88_handler(struct __interrupt_frame *frame) {
    __isr(88, frame);
}

__attribute__((interrupt)) static void __int89_handler(struct __interrupt_frame *frame) {
    __isr(89, frame);
}

__attribute__((interrupt)) static void __int90_handler(struct __interrupt_frame *frame) {
    __isr(90, frame);
}

__attribute__((interrupt)) static void __int91_handler(struct __interrupt_frame *frame) {
    __isr(91, frame);
}

__attribute__((interrupt)) static void __int92_handler(struct __interrupt_frame *frame) {
    __isr(92, frame);
}

__attribute__((interrupt)) static void __int93_handler(struct __interrupt_frame *frame) {
    __isr(93, frame);
}

__attribute__((interrupt)) static void __int94_handler(struct __interrupt_frame *frame) {
    __isr(94, frame);
}

__attribute__((interrupt)) static void __int95_handler(struct __interrupt_frame *frame) {
    __isr(95, frame);
}

__attribute__((interrupt)) static void __int96_handler(struct __interrupt_frame *frame) {
    __isr(96, frame);
}

__attribute__((interrupt)) static void __int97_handler(struct __interrupt_frame *frame) {
    __isr(97, frame);
}

__attribute__((interrupt)) static void __int98_handler(struct __interrupt_frame *frame) {
    __isr(98, frame);
}

__attribute__((interrupt)) static void __int99_handler(struct __interrupt_frame *frame) {
    __isr(99, frame);
}

__attribute__((interrupt)) static void __int100_handler(struct __interrupt_frame *frame) {
    __isr(100, frame);
}

__attribute__((interrupt)) static void __int101_handler(struct __interrupt_frame *frame) {
    __isr(101, frame);
}

__attribute__((interrupt)) static void __int102_handler(struct __interrupt_frame *frame) {
    __isr(102, frame);
}

__attribute__((interrupt)) static void __int103_handler(struct __interrupt_frame *frame) {
    __isr(103, frame);
}

__attribute__((interrupt)) static void __int104_handler(struct __interrupt_frame *frame) {
    __isr(104, frame);
}

__attribute__((interrupt)) static void __int105_handler(struct __interrupt_frame *frame) {
    __isr(105, frame);
}

__attribute__((interrupt)) static void __int106_handler(struct __interrupt_frame *frame) {
    __isr(106, frame);
}

__attribute__((interrupt)) static void __int107_handler(struct __interrupt_frame *frame) {
    __isr(107, frame);
}

__attribute__((interrupt)) static void __int108_handler(struct __interrupt_frame *frame) {
    __isr(108, frame);
}

__attribute__((interrupt)) static void __int109_handler(struct __interrupt_frame *frame) {
    __isr(109, frame);
}

__attribute__((interrupt)) static void __int110_handler(struct __interrupt_frame *frame) {
    __isr(110, frame);
}

__attribute__((interrupt)) static void __int111_handler(struct __interrupt_frame *frame) {
    __isr(111, frame);
}

__attribute__((interrupt)) static void __int112_handler(struct __interrupt_frame *frame) {
    __isr(112, frame);
}

__attribute__((interrupt)) static void __int113_handler(struct __interrupt_frame *frame) {
    __isr(113, frame);
}

__attribute__((interrupt)) static void __int114_handler(struct __interrupt_frame *frame) {
    __isr(114, frame);
}

__attribute__((interrupt)) static void __int115_handler(struct __interrupt_frame *frame) {
    __isr(115, frame);
}

__attribute__((interrupt)) static void __int116_handler(struct __interrupt_frame *frame) {
    __isr(116, frame);
}

__attribute__((interrupt)) static void __int117_handler(struct __interrupt_frame *frame) {
    __isr(117, frame);
}

__attribute__((interrupt)) static void __int118_handler(struct __interrupt_frame *frame) {
    __isr(118, frame);
}

__attribute__((interrupt)) static void __int119_handler(struct __interrupt_frame *frame) {
    __isr(119, frame);
}

__attribute__((interrupt)) static void __int120_handler(struct __interrupt_frame *frame) {
    __isr(120, frame);
}

__attribute__((interrupt)) static void __int121_handler(struct __interrupt_frame *frame) {
    __isr(121, frame);
}

__attribute__((interrupt)) static void __int122_handler(struct __interrupt_frame *frame) {
    __isr(122, frame);
}

__attribute__((interrupt)) static void __int123_handler(struct __interrupt_frame *frame) {
    __isr(123, frame);
}

__attribute__((interrupt)) static void __int124_handler(struct __interrupt_frame *frame) {
    __isr(124, frame);
}

__attribute__((interrupt)) static void __int125_handler(struct __interrupt_frame *frame) {
    __isr(125, frame);
}

__attribute__((interrupt)) static void __int126_handler(struct __interrupt_frame *frame) {
    __isr(126, frame);
}

__attribute__((interrupt)) static void __int127_handler(struct __interrupt_frame *frame) {
    __isr(127, frame);
}

__attribute__((interrupt)) static void __int128_handler(struct __interrupt_frame *frame) {
    __isr(128, frame);
}

__attribute__((interrupt)) static void __int129_handler(struct __interrupt_frame *frame) {
    __isr(129, frame);
}

__attribute__((interrupt)) static void __int130_handler(struct __interrupt_frame *frame) {
    __isr(130, frame);
}

__attribute__((interrupt)) static void __int131_handler(struct __interrupt_frame *frame) {
    __isr(131, frame);
}

__attribute__((interrupt)) static void __int132_handler(struct __interrupt_frame *frame) {
    __isr(132, frame);
}

__attribute__((interrupt)) static void __int133_handler(struct __interrupt_frame *frame) {
    __isr(133, frame);
}

__attribute__((interrupt)) static void __int134_handler(struct __interrupt_frame *frame) {
    __isr(134, frame);
}

__attribute__((interrupt)) static void __int135_handler(struct __interrupt_frame *frame) {
    __isr(135, frame);
}

__attribute__((interrupt)) static void __int136_handler(struct __interrupt_frame *frame) {
    __isr(136, frame);
}

__attribute__((interrupt)) static void __int137_handler(struct __interrupt_frame *frame) {
    __isr(137, frame);
}

__attribute__((interrupt)) static void __int138_handler(struct __interrupt_frame *frame) {
    __isr(138, frame);
}

__attribute__((interrupt)) static void __int139_handler(struct __interrupt_frame *frame) {
    __isr(139, frame);
}

__attribute__((interrupt)) static void __int140_handler(struct __interrupt_frame *frame) {
    __isr(140, frame);
}

__attribute__((interrupt)) static void __int141_handler(struct __interrupt_frame *frame) {
    __isr(141, frame);
}

__attribute__((interrupt)) static void __int142_handler(struct __interrupt_frame *frame) {
    __isr(142, frame);
}

__attribute__((interrupt)) static void __int143_handler(struct __interrupt_frame *frame) {
    __isr(143, frame);
}

__attribute__((interrupt)) static void __int144_handler(struct __interrupt_frame *frame) {
    __isr(144, frame);
}

__attribute__((interrupt)) static void __int145_handler(struct __interrupt_frame *frame) {
    __isr(145, frame);
}

__attribute__((interrupt)) static void __int146_handler(struct __interrupt_frame *frame) {
    __isr(146, frame);
}

__attribute__((interrupt)) static void __int147_handler(struct __interrupt_frame *frame) {
    __isr(147, frame);
}

__attribute__((interrupt)) static void __int148_handler(struct __interrupt_frame *frame) {
    __isr(148, frame);
}

__attribute__((interrupt)) static void __int149_handler(struct __interrupt_frame *frame) {
    __isr(149, frame);
}

__attribute__((interrupt)) static void __int150_handler(struct __interrupt_frame *frame) {
    __isr(150, frame);
}

__attribute__((interrupt)) static void __int151_handler(struct __interrupt_frame *frame) {
    __isr(151, frame);
}

__attribute__((interrupt)) static void __int152_handler(struct __interrupt_frame *frame) {
    __isr(152, frame);
}

__attribute__((interrupt)) static void __int153_handler(struct __interrupt_frame *frame) {
    __isr(153, frame);
}

__attribute__((interrupt)) static void __int154_handler(struct __interrupt_frame *frame) {
    __isr(154, frame);
}

__attribute__((interrupt)) static void __int155_handler(struct __interrupt_frame *frame) {
    __isr(155, frame);
}

__attribute__((interrupt)) static void __int156_handler(struct __interrupt_frame *frame) {
    __isr(156, frame);
}

__attribute__((interrupt)) static void __int157_handler(struct __interrupt_frame *frame) {
    __isr(157, frame);
}

__attribute__((interrupt)) static void __int158_handler(struct __interrupt_frame *frame) {
    __isr(158, frame);
}

__attribute__((interrupt)) static void __int159_handler(struct __interrupt_frame *frame) {
    __isr(159, frame);
}

__attribute__((interrupt)) static void __int160_handler(struct __interrupt_frame *frame) {
    __isr(160, frame);
}

__attribute__((interrupt)) static void __int161_handler(struct __interrupt_frame *frame) {
    __isr(161, frame);
}

__attribute__((interrupt)) static void __int162_handler(struct __interrupt_frame *frame) {
    __isr(162, frame);
}

__attribute__((interrupt)) static void __int163_handler(struct __interrupt_frame *frame) {
    __isr(163, frame);
}

__attribute__((interrupt)) static void __int164_handler(struct __interrupt_frame *frame) {
    __isr(164, frame);
}

__attribute__((interrupt)) static void __int165_handler(struct __interrupt_frame *frame) {
    __isr(165, frame);
}

__attribute__((interrupt)) static void __int166_handler(struct __interrupt_frame *frame) {
    __isr(166, frame);
}

__attribute__((interrupt)) static void __int167_handler(struct __interrupt_frame *frame) {
    __isr(167, frame);
}

__attribute__((interrupt)) static void __int168_handler(struct __interrupt_frame *frame) {
    __isr(168, frame);
}

__attribute__((interrupt)) static void __int169_handler(struct __interrupt_frame *frame) {
    __isr(169, frame);
}

__attribute__((interrupt)) static void __int170_handler(struct __interrupt_frame *frame) {
    __isr(170, frame);
}

__attribute__((interrupt)) static void __int171_handler(struct __interrupt_frame *frame) {
    __isr(171, frame);
}

__attribute__((interrupt)) static void __int172_handler(struct __interrupt_frame *frame) {
    __isr(172, frame);
}

__attribute__((interrupt)) static void __int173_handler(struct __interrupt_frame *frame) {
    __isr(173, frame);
}

__attribute__((interrupt)) static void __int174_handler(struct __interrupt_frame *frame) {
    __isr(174, frame);
}

__attribute__((interrupt)) static void __int175_handler(struct __interrupt_frame *frame) {
    __isr(175, frame);
}

__attribute__((interrupt)) static void __int176_handler(struct __interrupt_frame *frame) {
    __isr(176, frame);
}

__attribute__((interrupt)) static void __int177_handler(struct __interrupt_frame *frame) {
    __isr(177, frame);
}

__attribute__((interrupt)) static void __int178_handler(struct __interrupt_frame *frame) {
    __isr(178, frame);
}

__attribute__((interrupt)) static void __int179_handler(struct __interrupt_frame *frame) {
    __isr(179, frame);
}

__attribute__((interrupt)) static void __int180_handler(struct __interrupt_frame *frame) {
    __isr(180, frame);
}

__attribute__((interrupt)) static void __int181_handler(struct __interrupt_frame *frame) {
    __isr(181, frame);
}

__attribute__((interrupt)) static void __int182_handler(struct __interrupt_frame *frame) {
    __isr(182, frame);
}

__attribute__((interrupt)) static void __int183_handler(struct __interrupt_frame *frame) {
    __isr(183, frame);
}

__attribute__((interrupt)) static void __int184_handler(struct __interrupt_frame *frame) {
    __isr(184, frame);
}

__attribute__((interrupt)) static void __int185_handler(struct __interrupt_frame *frame) {
    __isr(185, frame);
}

__attribute__((interrupt)) static void __int186_handler(struct __interrupt_frame *frame) {
    __isr(186, frame);
}

__attribute__((interrupt)) static void __int187_handler(struct __interrupt_frame *frame) {
    __isr(187, frame);
}

__attribute__((interrupt)) static void __int188_handler(struct __interrupt_frame *frame) {
    __isr(188, frame);
}

__attribute__((interrupt)) static void __int189_handler(struct __interrupt_frame *frame) {
    __isr(189, frame);
}

__attribute__((interrupt)) static void __int190_handler(struct __interrupt_frame *frame) {
    __isr(190, frame);
}

__attribute__((interrupt)) static void __int191_handler(struct __interrupt_frame *frame) {
    __isr(191, frame);
}

__attribute__((interrupt)) static void __int192_handler(struct __interrupt_frame *frame) {
    __isr(192, frame);
}

__attribute__((interrupt)) static void __int193_handler(struct __interrupt_frame *frame) {
    __isr(193, frame);
}

__attribute__((interrupt)) static void __int194_handler(struct __interrupt_frame *frame) {
    __isr(194, frame);
}

__attribute__((interrupt)) static void __int195_handler(struct __interrupt_frame *frame) {
    __isr(195, frame);
}

__attribute__((interrupt)) static void __int196_handler(struct __interrupt_frame *frame) {
    __isr(196, frame);
}

__attribute__((interrupt)) static void __int197_handler(struct __interrupt_frame *frame) {
    __isr(197, frame);
}

__attribute__((interrupt)) static void __int198_handler(struct __interrupt_frame *frame) {
    __isr(198, frame);
}

__attribute__((interrupt)) static void __int199_handler(struct __interrupt_frame *frame) {
    __isr(199, frame);
}

__attribute__((interrupt)) static void __int200_handler(struct __interrupt_frame *frame) {
    __isr(200, frame);
}

__attribute__((interrupt)) static void __int201_handler(struct __interrupt_frame *frame) {
    __isr(201, frame);
}

__attribute__((interrupt)) static void __int202_handler(struct __interrupt_frame *frame) {
    __isr(202, frame);
}

__attribute__((interrupt)) static void __int203_handler(struct __interrupt_frame *frame) {
    __isr(203, frame);
}

__attribute__((interrupt)) static void __int204_handler(struct __interrupt_frame *frame) {
    __isr(204, frame);
}

__attribute__((interrupt)) static void __int205_handler(struct __interrupt_frame *frame) {
    __isr(205, frame);
}

__attribute__((interrupt)) static void __int206_handler(struct __interrupt_frame *frame) {
    __isr(206, frame);
}

__attribute__((interrupt)) static void __int207_handler(struct __interrupt_frame *frame) {
    __isr(207, frame);
}

__attribute__((interrupt)) static void __int208_handler(struct __interrupt_frame *frame) {
    __isr(208, frame);
}

__attribute__((interrupt)) static void __int209_handler(struct __interrupt_frame *frame) {
    __isr(209, frame);
}

__attribute__((interrupt)) static void __int210_handler(struct __interrupt_frame *frame) {
    __isr(210, frame);
}

__attribute__((interrupt)) static void __int211_handler(struct __interrupt_frame *frame) {
    __isr(211, frame);
}

__attribute__((interrupt)) static void __int212_handler(struct __interrupt_frame *frame) {
    __isr(212, frame);
}

__attribute__((interrupt)) static void __int213_handler(struct __interrupt_frame *frame) {
    __isr(213, frame);
}

__attribute__((interrupt)) static void __int214_handler(struct __interrupt_frame *frame) {
    __isr(214, frame);
}

__attribute__((interrupt)) static void __int215_handler(struct __interrupt_frame *frame) {
    __isr(215, frame);
}

__attribute__((interrupt)) static void __int216_handler(struct __interrupt_frame *frame) {
    __isr(216, frame);
}

__attribute__((interrupt)) static void __int217_handler(struct __interrupt_frame *frame) {
    __isr(217, frame);
}

__attribute__((interrupt)) static void __int218_handler(struct __interrupt_frame *frame) {
    __isr(218, frame);
}

__attribute__((interrupt)) static void __int219_handler(struct __interrupt_frame *frame) {
    __isr(219, frame);
}

__attribute__((interrupt)) static void __int220_handler(struct __interrupt_frame *frame) {
    __isr(220, frame);
}

__attribute__((interrupt)) static void __int221_handler(struct __interrupt_frame *frame) {
    __isr(221, frame);
}

__attribute__((interrupt)) static void __int222_handler(struct __interrupt_frame *frame) {
    __isr(222, frame);
}

__attribute__((interrupt)) static void __int223_handler(struct __interrupt_frame *frame) {
    __isr(223, frame);
}

__attribute__((interrupt)) static void __int224_handler(struct __interrupt_frame *frame) {
    __isr(224, frame);
}

__attribute__((interrupt)) static void __int225_handler(struct __interrupt_frame *frame) {
    __isr(225, frame);
}

__attribute__((interrupt)) static void __int226_handler(struct __interrupt_frame *frame) {
    __isr(226, frame);
}

__attribute__((interrupt)) static void __int227_handler(struct __interrupt_frame *frame) {
    __isr(227, frame);
}

__attribute__((interrupt)) static void __int228_handler(struct __interrupt_frame *frame) {
    __isr(228, frame);
}

__attribute__((interrupt)) static void __int229_handler(struct __interrupt_frame *frame) {
    __isr(229, frame);
}

__attribute__((interrupt)) static void __int230_handler(struct __interrupt_frame *frame) {
    __isr(230, frame);
}

__attribute__((interrupt)) static void __int231_handler(struct __interrupt_frame *frame) {
    __isr(231, frame);
}

__attribute__((interrupt)) static void __int232_handler(struct __interrupt_frame *frame) {
    __isr(232, frame);
}

__attribute__((interrupt)) static void __int233_handler(struct __interrupt_frame *frame) {
    __isr(233, frame);
}

__attribute__((interrupt)) static void __int234_handler(struct __interrupt_frame *frame) {
    __isr(234, frame);
}

__attribute__((interrupt)) static void __int235_handler(struct __interrupt_frame *frame) {
    __isr(235, frame);
}

__attribute__((interrupt)) static void __int236_handler(struct __interrupt_frame *frame) {
    __isr(236, frame);
}

__attribute__((interrupt)) static void __int237_handler(struct __interrupt_frame *frame) {
    __isr(237, frame);
}

__attribute__((interrupt)) static void __int238_handler(struct __interrupt_frame *frame) {
    __isr(238, frame);
}

__attribute__((interrupt)) static void __int239_handler(struct __interrupt_frame *frame) {
    __isr(239, frame);
}

__attribute__((interrupt)) static void __int240_handler(struct __interrupt_frame *frame) {
    __isr(240, frame);
}

__attribute__((interrupt)) static void __int241_handler(struct __interrupt_frame *frame) {
    __isr(241, frame);
}

__attribute__((interrupt)) static void __int242_handler(struct __interrupt_frame *frame) {
    __isr(242, frame);
}

__attribute__((interrupt)) static void __int243_handler(struct __interrupt_frame *frame) {
    __isr(243, frame);
}

__attribute__((interrupt)) static void __int244_handler(struct __interrupt_frame *frame) {
    __isr(244, frame);
}

__attribute__((interrupt)) static void __int245_handler(struct __interrupt_frame *frame) {
    __isr(245, frame);
}

__attribute__((interrupt)) static void __int246_handler(struct __interrupt_frame *frame) {
    __isr(246, frame);
}

__attribute__((interrupt)) static void __int247_handler(struct __interrupt_frame *frame) {
    __isr(247, frame);
}

__attribute__((interrupt)) static void __int248_handler(struct __interrupt_frame *frame) {
    __isr(248, frame);
}

__attribute__((interrupt)) static void __int249_handler(struct __interrupt_frame *frame) {
    __isr(249, frame);
}

__attribute__((interrupt)) static void __int250_handler(struct __interrupt_frame *frame) {
    __isr(250, frame);
}

__attribute__((interrupt)) static void __int251_handler(struct __interrupt_frame *frame) {
    __isr(251, frame);
}

__attribute__((interrupt)) static void __int252_handler(struct __interrupt_frame *frame) {
    __isr(252, frame);
}

__attribute__((interrupt)) static void __int253_handler(struct __interrupt_frame *frame) {
    __isr(253, frame);
}

__attribute__((interrupt)) static void __int254_handler(struct __interrupt_frame *frame) {
    __isr(254, frame);
}

__attribute__((interrupt)) static void __int255_handler(struct __interrupt_frame *frame) {
    __isr(255, frame);
}

void (* isrs[256])(struct __interrupt_frame *) = {
    &__int0_handler,
    &__int1_handler,
    &__int2_handler,
    &__int3_handler,
    &__int4_handler,
    &__int5_handler,
    &__int6_handler,
    &__int7_handler,
    &__int8_handler,
    &__int9_handler,
    &__int10_handler,
    &__int11_handler,
    &__int12_handler,
    &__int13_handler,
    &__int14_handler,
    &__int15_handler,
    &__int16_handler,
    &__int17_handler,
    &__int18_handler,
    &__int19_handler,
    &__int20_handler,
    &__int21_handler,
    &__int22_handler,
    &__int23_handler,
    &__int24_handler,
    &__int25_handler,
    &__int26_handler,
    &__int27_handler,
    &__int28_handler,
    &__int29_handler,
    &__int30_handler,
    &__int31_handler,
    &__int32_handler,
    &__int33_handler,
    &__int34_handler,
    &__int35_handler,
    &__int36_handler,
    &__int37_handler,
    &__int38_handler,
    &__int39_handler,
    &__int40_handler,
    &__int41_handler,
    &__int42_handler,
    &__int43_handler,
    &__int44_handler,
    &__int45_handler,
    &__int46_handler,
    &__int47_handler,
    &__int48_handler,
    &__int49_handler,
    &__int50_handler,
    &__int51_handler,
    &__int52_handler,
    &__int53_handler,
    &__int54_handler,
    &__int55_handler,
    &__int56_handler,
    &__int57_handler,
    &__int58_handler,
    &__int59_handler,
    &__int60_handler,
    &__int61_handler,
    &__int62_handler,
    &__int63_handler,
    &__int64_handler,
    &__int65_handler,
    &__int66_handler,
    &__int67_handler,
    &__int68_handler,
    &__int69_handler,
    &__int70_handler,
    &__int71_handler,
    &__int72_handler,
    &__int73_handler,
    &__int74_handler,
    &__int75_handler,
    &__int76_handler,
    &__int77_handler,
    &__int78_handler,
    &__int79_handler,
    &__int80_handler,
    &__int81_handler,
    &__int82_handler,
    &__int83_handler,
    &__int84_handler,
    &__int85_handler,
    &__int86_handler,
    &__int87_handler,
    &__int88_handler,
    &__int89_handler,
    &__int90_handler,
    &__int91_handler,
    &__int92_handler,
    &__int93_handler,
    &__int94_handler,
    &__int95_handler,
    &__int96_handler,
    &__int97_handler,
    &__int98_handler,
    &__int99_handler,
    &__int100_handler,
    &__int101_handler,
    &__int102_handler,
    &__int103_handler,
    &__int104_handler,
    &__int105_handler,
    &__int106_handler,
    &__int107_handler,
    &__int108_handler,
    &__int109_handler,
    &__int110_handler,
    &__int111_handler,
    &__int112_handler,
    &__int113_handler,
    &__int114_handler,
    &__int115_handler,
    &__int116_handler,
    &__int117_handler,
    &__int118_handler,
    &__int119_handler,
    &__int120_handler,
    &__int121_handler,
    &__int122_handler,
    &__int123_handler,
    &__int124_handler,
    &__int125_handler,
    &__int126_handler,
    &__int127_handler,
    &__int128_handler,
    &__int129_handler,
    &__int130_handler,
    &__int131_handler,
    &__int132_handler,
    &__int133_handler,
    &__int134_handler,
    &__int135_handler,
    &__int136_handler,
    &__int137_handler,
    &__int138_handler,
    &__int139_handler,
    &__int140_handler,
    &__int141_handler,
    &__int142_handler,
    &__int143_handler,
    &__int144_handler,
    &__int145_handler,
    &__int146_handler,
    &__int147_handler,
    &__int148_handler,
    &__int149_handler,
    &__int150_handler,
    &__int151_handler,
    &__int152_handler,
    &__int153_handler,
    &__int154_handler,
    &__int155_handler,
    &__int156_handler,
    &__int157_handler,
    &__int158_handler,
    &__int159_handler,
    &__int160_handler,
    &__int161_handler,
    &__int162_handler,
    &__int163_handler,
    &__int164_handler,
    &__int165_handler,
    &__int166_handler,
    &__int167_handler,
    &__int168_handler,
    &__int169_handler,
    &__int170_handler,
    &__int171_handler,
    &__int172_handler,
    &__int173_handler,
    &__int174_handler,
    &__int175_handler,
    &__int176_handler,
    &__int177_handler,
    &__int178_handler,
    &__int179_handler,
    &__int180_handler,
    &__int181_handler,
    &__int182_handler,
    &__int183_handler,
    &__int184_handler,
    &__int185_handler,
    &__int186_handler,
    &__int187_handler,
    &__int188_handler,
    &__int189_handler,
    &__int190_handler,
    &__int191_handler,
    &__int192_handler,
    &__int193_handler,
    &__int194_handler,
    &__int195_handler,
    &__int196_handler,
    &__int197_handler,
    &__int198_handler,
    &__int199_handler,
    &__int200_handler,
    &__int201_handler,
    &__int202_handler,
    &__int203_handler,
    &__int204_handler,
    &__int205_handler,
    &__int206_handler,
    &__int207_handler,
    &__int208_handler,
    &__int209_handler,
    &__int210_handler,
    &__int211_handler,
    &__int212_handler,
    &__int213_handler,
    &__int214_handler,
    &__int215_handler,
    &__int216_handler,
    &__int217_handler,
    &__int218_handler,
    &__int219_handler,
    &__int220_handler,
    &__int221_handler,
    &__int222_handler,
    &__int223_handler,
    &__int224_handler,
    &__int225_handler,
    &__int226_handler,
    &__int227_handler,
    &__int228_handler,
    &__int229_handler,
    &__int230_handler,
    &__int231_handler,
    &__int232_handler,
    &__int233_handler,
    &__int234_handler,
    &__int235_handler,
    &__int236_handler,
    &__int237_handler,
    &__int238_handler,
    &__int239_handler,
    &__int240_handler,
    &__int241_handler,
    &__int242_handler,
    &__int243_handler,
    &__int244_handler,
    &__int245_handler,
    &__int246_handler,
    &__int247_handler,
    &__int248_handler,
    &__int249_handler,
    &__int250_handler,
    &__int251_handler,
    &__int252_handler,
    &__int253_handler,
    &__int254_handler,
    &__int255_handler,
};

void __isr_init(void) {
    for (uint32_t i = 0; i < 256; ++i)
        __set_handler(i, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, isrs[i]);
}