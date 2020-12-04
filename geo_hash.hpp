#pragma once

#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <tuple>

namespace geo_hash {
    using std::string;
    template<typename T> using vector = std::vector<T>;

    void test();
    string encode(const double lat, const double lon);

    namespace geo_hash_util{
        string to_binary_expression(const double val, const double min_initial, const double max_initial, const double precision);
        string zip_strings(const string s1, const string s2);
        string binary_to_base32(const string s);
        string extract_i_th_n_characters(const string s, const int i, const int n, const char c);
        char decimal_to_base32(const int n);
    }

    void test(){
        // https://geohash.measurement.earth/
        vector<std::tuple<double, double, string, bool>> tests = {
            std::make_tuple(37.34999657, 140.33952713, "xneu3x25", true)
            , std::make_tuple(35.66943169, 139.54851151, "xn74zt2j", true)
            , std::make_tuple(35.55630684, 136.87883377, "xn366bb4", true)
            , std::make_tuple(33.11356544, 131.28679276, "wvvk7s0h", true)
            , std::make_tuple(42.98375130, 142.58073807, "xptks8bn", true)
            , std::make_tuple(39.32753563, -83.49626541, "dnuxezrg", true)
            , std::make_tuple(-37.32819557, 144.14079666, "r1qe8h85", true)
            , std::make_tuple(-37.32819557, 144.14079666, "hogehoge", false)
            , std::make_tuple(37.34999657, 140.33952713, "fugafuga", false)
        };
        for(auto& test : tests){
            double lat = std::get<0>(test);
            double lon = std::get<1>(test);
            string geo_hash = std::get<2>(test);
            bool expected = std::get<3>(test);

            string result;
            if((encode(lat, lon).substr(0, 8) == geo_hash)  == expected){
                result = "Passed.";
            }else{
                result = "Failed.";
            }
            std::cout << "lat: " + std::to_string(lat) << ", lon: " + std::to_string(lon) << ", geo_hash: " + geo_hash << ", result: " + result << std::endl;
        }
    }


    string encode(const double lat, const double lon){
        if(std::abs(lon) > 180. || std::abs(lat) > 90.){
            throw std::invalid_argument("緯度経度の値が不正. lon : " + std::to_string(lon) + ", lat : " + std::to_string(lat));
        }
        const string lat_bin = geo_hash_util::to_binary_expression(lat, -90, 90, 1e-6);
        const string lon_bin = geo_hash_util::to_binary_expression(lon, -180, 180, 1e-6);
        const string zipped = geo_hash_util::zip_strings(lon_bin, lat_bin);
        return geo_hash_util::binary_to_base32(zipped);
    }

    namespace geo_hash_util{
        string to_binary_expression(const double val, const double min_initial, const double max_initial, const double precision){
            // valについて求める精度まで2分探索を行い, 得られた経路を2進数の文字列として返す
            double max = max_initial;
            double min = min_initial;
            string result = "";

            do {
                const double mid = 0.5 * (max + min);
                const bool is_greater_than_mid = val > mid;
                result += std::to_string(is_greater_than_mid);

                if(is_greater_than_mid){
                    min = mid;
                }else{
                    max = mid;
                }
            }while(std::abs(max - min) > precision);
    
            return result;
        }


        string zip_strings(const string s1, const string s2){
            // abcd, 1234 => a1b2c3d4的な
            const int s1_size = s1.size();
            const int s2_size = s2.size();
            int capacity = 0;

            // s1, s2のsizeから返り値のsizeを求める
            if(s1_size == 0){
                return "";
            }else if(s1_size == s2_size || s1_size == s2_size + 1){
                capacity = s1_size + s2_size;
            }else if(s1_size > s2_size + 1){
                capacity = 2 * s2_size;
            }else{  // s1_size < s2_size
                capacity = 2 * s1_size;
            }

            string result(capacity, 'a');

            for(int i = 0 ; i < capacity ; ++i){
                if(i % 2 == 0){
                    result.at(i) = s1.at(i / 2);
                }else{
                    result.at(i) = s2.at(i / 2);
                }
            }

            return result;
        }

        string binary_to_base32(const string s){
            string cp_s = s;
            std::sort(cp_s.begin(), cp_s.end());
            cp_s.erase(std::unique(cp_s.begin(), cp_s.end()), cp_s.end());
            if(cp_s != "01"){
                throw std::invalid_argument("binary_to_base32()の引数がbinaryではない. s : " + s);
            }

            const int capacity = ceil(s.size() / 5.);
            string result(capacity, '_');

            for(int i = 0 ; i < capacity ; ++i){
                const string part_binary = extract_i_th_n_characters(s, i, 5, '0');
                const int part_decimal = std::stoi(part_binary, nullptr, 2);  // 2進数として解釈しintに変換
                result.at(i) = decimal_to_base32(part_decimal);
            }
    
            return result;
        }


        string extract_i_th_n_characters(const string s, const int i, const int n, const char c){
            // sをn文字毎に分割したi番目を返す (i >= 0). 結果がn文字に見たない場合はcで埋める.
            string result;

            if((i * n) + n <= s.size()){
                result = s.substr(i * n, n);
            }else{
                result = s.substr(i * n);
            }
            if(result.size() < n){
                result += string(n - result.size(), c);
            }

            return result;
        }


        char decimal_to_base32(const int n){
            if(n < 0 || 32 < n){
                throw std::invalid_argument("decimal_to_base32()の引数が0から32ではない. n : " + std::to_string(n));
            }

            char result;

            if(n <= 9){
                result = '0' + n;
            }else if(n <= 16){
                result = char(int('a') + (n - 9));
            }else if(n <= 18){
                result = char(int('a') + (n - 9) + 1);
            }else if(n <= 20){
                result = char(int('a') + (n - 9) + 2);
            }else{
                result = char(int('a') + (n - 9) + 3);
            }

            return result;
        }
    }
}