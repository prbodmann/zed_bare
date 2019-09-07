set_property IOSTANDARD LVCMOS33 [get_ports tx]
set_property PACKAGE_PIN AA11 [get_ports tx] ;#JA2

set_property IOSTANDARD LVCMOS33 [get_ports rx]
set_property PACKAGE_PIN Y10 [get_ports rx] ;#JA3

set_property BITSTREAM.SEU.ESSENTIALBITS yes [current_design]

# Compression of bitstreams using Per-Frame CRC is not supported.
set_property BITSTREAM.GENERAL.COMPRESS FALSE [current_design]
set_property BITSTREAM.GENERAL.CRC ENABLE [current_design]
set_property BITSTREAM.GENERAL.PERFRAMECRC yes [current_design]

set_property POST_CRC ENABLE [current_design]
set_property POST_CRC_SOURCE PRE_COMPUTED [current_design]
set_property POST_CRC_ACTION CORRECT_AND_CONTINUE [current_design]
set_property POST_CRC_INIT_FLAG DISABLE [current_design]
set_property POST_CRC_FREQ {50} [current_design]

set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
