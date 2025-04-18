#include <fram.hpp>

#include <record.hpp>
#include <sensor/factory_calibration.hpp>

#include <memory>
#include <cstdint>
#include <vector>

namespace cgm {

FRAM_data::FRAM_data(const std::shared_ptr<FactoryCalibration> calibration, const std::vector<uint8_t>& fram) {
    if (fram.size() == 0) {
        error = true;
        return;
    }

    /// HEADER ///
    header_crc   = {fram[0], fram[1]};
    sensor_state = fram[4];

    /// BODY ///
    body_crc       = {fram[24], fram[25]};
    trend_index    = fram[26];
    historic_index = fram[27];

    trend_records.reserve(sizeof(fram_record) * 16); // 16 trend records (6 bytes each) for the last 16 minutes
    trend_records.push_back(fram_record(calibration, {fram[28], fram[29], fram[30], fram[31], fram[32], fram[33]}));
    // trend_records.push_back(fram_record(calibration, {0x38, 0x60, 0x00, 0x3C, 0xB9, 0x00})); // test data: raw_glucose = 1564, has_error = false, raw_temp = 7484, temp_adjustment = 8, negative = false
    trend_records.push_back(fram_record(calibration, {fram[34], fram[35], fram[36], fram[37], fram[38], fram[39]}));
    trend_records.push_back(fram_record(calibration, {fram[40], fram[41], fram[42], fram[43], fram[44], fram[45]}));
    trend_records.push_back(fram_record(calibration, {fram[46], fram[47], fram[48], fram[49], fram[50], fram[51]}));
    trend_records.push_back(fram_record(calibration, {fram[52], fram[53], fram[54], fram[55], fram[56], fram[57]}));
    trend_records.push_back(fram_record(calibration, {fram[58], fram[59], fram[60], fram[61], fram[62], fram[63]}));
    trend_records.push_back(fram_record(calibration, {fram[64], fram[65], fram[66], fram[67], fram[68], fram[69]}));
    trend_records.push_back(fram_record(calibration, {fram[70], fram[71], fram[72], fram[73], fram[74], fram[75]}));
    trend_records.push_back(fram_record(calibration, {fram[76], fram[77], fram[78], fram[79], fram[80], fram[81]}));
    trend_records.push_back(fram_record(calibration, {fram[82], fram[83], fram[84], fram[85], fram[86], fram[87]}));
    trend_records.push_back(fram_record(calibration, {fram[88], fram[89], fram[90], fram[91], fram[92], fram[93]}));
    trend_records.push_back(fram_record(calibration, {fram[94], fram[95], fram[96], fram[97], fram[98], fram[99]}));
    trend_records.push_back(fram_record(calibration, {fram[100], fram[101], fram[102], fram[103], fram[104], fram[105]}));
    trend_records.push_back(fram_record(calibration, {fram[106], fram[107], fram[108], fram[109], fram[110], fram[111]}));
    trend_records.push_back(fram_record(calibration, {fram[112], fram[113], fram[114], fram[115], fram[116], fram[117]}));
    trend_records.push_back(fram_record(calibration, {fram[118], fram[119], fram[120], fram[121], fram[122], fram[123]}));

    historic_records.reserve(sizeof(fram_record) * 32); // 32 historic records (15 minutes each) for the last 8 hours
    historic_records.push_back(fram_record(calibration, {fram[124], fram[125], fram[126], fram[127], fram[128], fram[129]}));
    historic_records.push_back(fram_record(calibration, {fram[130], fram[131], fram[132], fram[133], fram[134], fram[135]}));
    historic_records.push_back(fram_record(calibration, {fram[136], fram[137], fram[138], fram[139], fram[140], fram[141]}));
    historic_records.push_back(fram_record(calibration, {fram[142], fram[143], fram[144], fram[145], fram[146], fram[147]}));
    historic_records.push_back(fram_record(calibration, {fram[148], fram[149], fram[150], fram[151], fram[152], fram[153]}));
    historic_records.push_back(fram_record(calibration, {fram[154], fram[155], fram[156], fram[157], fram[158], fram[159]}));
    historic_records.push_back(fram_record(calibration, {fram[160], fram[161], fram[162], fram[163], fram[164], fram[165]}));
    historic_records.push_back(fram_record(calibration, {fram[166], fram[167], fram[168], fram[169], fram[170], fram[171]}));
    historic_records.push_back(fram_record(calibration, {fram[172], fram[173], fram[174], fram[175], fram[176], fram[177]}));
    historic_records.push_back(fram_record(calibration, {fram[178], fram[179], fram[180], fram[181], fram[182], fram[183]}));
    historic_records.push_back(fram_record(calibration, {fram[184], fram[185], fram[186], fram[187], fram[188], fram[189]}));
    historic_records.push_back(fram_record(calibration, {fram[190], fram[191], fram[192], fram[193], fram[194], fram[195]}));
    historic_records.push_back(fram_record(calibration, {fram[196], fram[197], fram[198], fram[199], fram[200], fram[201]}));
    historic_records.push_back(fram_record(calibration, {fram[202], fram[203], fram[204], fram[205], fram[206], fram[207]}));
    historic_records.push_back(fram_record(calibration, {fram[208], fram[209], fram[210], fram[211], fram[212], fram[213]}));
    historic_records.push_back(fram_record(calibration, {fram[214], fram[215], fram[216], fram[217], fram[218], fram[219]}));
    historic_records.push_back(fram_record(calibration, {fram[220], fram[221], fram[222], fram[223], fram[224], fram[225]}));
    historic_records.push_back(fram_record(calibration, {fram[226], fram[227], fram[228], fram[229], fram[230], fram[231]}));
    historic_records.push_back(fram_record(calibration, {fram[232], fram[233], fram[234], fram[235], fram[236], fram[237]}));
    historic_records.push_back(fram_record(calibration, {fram[238], fram[239], fram[240], fram[241], fram[242], fram[243]}));
    historic_records.push_back(fram_record(calibration, {fram[244], fram[245], fram[246], fram[247], fram[248], fram[249]}));
    historic_records.push_back(fram_record(calibration, {fram[250], fram[251], fram[252], fram[253], fram[254], fram[255]}));
    historic_records.push_back(fram_record(calibration, {fram[256], fram[257], fram[258], fram[259], fram[260], fram[261]}));
    historic_records.push_back(fram_record(calibration, {fram[262], fram[263], fram[264], fram[265], fram[266], fram[267]}));
    historic_records.push_back(fram_record(calibration, {fram[268], fram[269], fram[270], fram[271], fram[272], fram[273]}));
    historic_records.push_back(fram_record(calibration, {fram[274], fram[275], fram[276], fram[277], fram[278], fram[279]}));
    historic_records.push_back(fram_record(calibration, {fram[280], fram[281], fram[282], fram[283], fram[284], fram[285]}));
    historic_records.push_back(fram_record(calibration, {fram[286], fram[287], fram[288], fram[289], fram[290], fram[291]}));
    historic_records.push_back(fram_record(calibration, {fram[292], fram[293], fram[294], fram[295], fram[296], fram[297]}));
    historic_records.push_back(fram_record(calibration, {fram[298], fram[299], fram[300], fram[301], fram[302], fram[303]}));
    historic_records.push_back(fram_record(calibration, {fram[304], fram[305], fram[306], fram[307], fram[308], fram[309]}));
    historic_records.push_back(fram_record(calibration, {fram[310], fram[311], fram[312], fram[313], fram[314], fram[315]}));

    age = {fram[316], fram[317]};

    /// FOOTER ///
    footer_crc = {fram[320], fram[321]};
    max_life = {fram[326], fram[327]};
}

} // namespace cgm
