
namespace futa{
    void get_ccl_data(const std::vector<std::vector<Cell>>& table, std::vector<int>& data, int& W){
        W = table[0].size();
        //vector<int> data;
        for (const auto& xs : table) {
            for (const auto& x : xs) {
                data.push_back((x == Cell::Obstacle || x == Cell::Occupied) ? 1 : 0);
            }
        }
    }

    void calc_ccl(std::vector<int>& data, const int W, std::vector<int>& result, vector<int>& spaces){
        //std::vector<int> result(ccl.ccl(data, W));
        nf_ccl::CCL ccl;
        result = ccl.ccl(data, W);

        //std::vector<int> memo;
        for (int i = 0; i < static_cast<int>(result.size()) / W; i++) {
            for (int j = 0; j < W; j++) {
                if (data[i*W+j] == 0 && find(spaces.begin(), spaces.end(), result[i*W+j]) == spaces.end()) {
                    spaces.push_back(result[i*W+j]);
                }
            }
        }
    }

    void check_ccl(const std::vector<std::vector<Cell>>& table, vector<int>& result, vector<int>& spaces){
        int W;
        vector<int> data;
        result.clear();
        spaces.clear();
        get_ccl_data(table, data, W);
        calc_ccl(data, W, result, spaces);
    }
}

void getLabeledPoints( const vector<vector<Cell>> table, vector<vector<boardloader::Point>>& labeledPoints ){
    std::vector<boardloader::Point> area;
    //int H = table.size();
    int W = table[0].size();

    vector<int> ccl_data, ccl_spaces;
    futa::check_ccl( table, ccl_data, ccl_spaces );

    for (const auto& space : ccl_spaces) {
        area.clear();
        for (int i = 0; i < ccl_data.size(); i++) {
            if (ccl_data[i] == space) {
                //area.push_back(Point(H - i / W - 1, i % W));
                area.push_back(boardloader::Point(i / W, i % W));
            }
        }
        labeledPoints.push_back( area );
    }
}