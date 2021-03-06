#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <algorithm>
// For External Library
#include <torch/torch.h>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <png++/png.hpp>
// For Original Header
#include "transforms.hpp"
#include "datasets.hpp"

namespace fs = boost::filesystem;


// -----------------------------------------------
// namespace{datasets} -> function{RGB_Loader}
// -----------------------------------------------
cv::Mat datasets::RGB_Loader(std::string &path){
    cv::Mat BGR, RGB;
    BGR = cv::imread(path, cv::IMREAD_COLOR | cv::IMREAD_ANYDEPTH);  // path ===> color image {B,G,R}
    cv::cvtColor(BGR, RGB, cv::COLOR_BGR2RGB);  // {0,1,2} = {B,G,R} ===> {0,1,2} = {R,G,B}
    return RGB.clone();
}


// -----------------------------------------------
// namespace{datasets} -> function{Index_Loader}
// -----------------------------------------------
cv::Mat datasets::Index_Loader(std::string &path){

    size_t i, j;    
    size_t width, height;
    cv::Mat Index;

    png::image<png::index_pixel> Index_png(path);  // path ===> index image

    width = Index_png.get_width();
    height = Index_png.get_height();
    Index = cv::Mat(cv::Size(width, height), CV_32SC1);
    for (j = 0; j < height; j++){
        for (i = 0; i < width; i++){
            Index.at<int>(j, i) = (int)Index_png[j][i];
        }
    }

    return Index.clone();

}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderWithPaths} -> constructor
// -------------------------------------------------------------------------
datasets::ImageFolderWithPaths::ImageFolderWithPaths(const std::string root, std::vector<transforms::Compose*> &transform_){
    fs::path ROOT = fs::path(root);
    for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
        if (!fs::is_directory(p)){
            std::stringstream rpath, fname;
            rpath << p.path().string();
            fname << p.path().filename().string();
            this->paths.push_back(rpath.str());
            this->fnames.push_back(fname.str());
        }
    }
    std::sort(this->paths.begin(), this->paths.end());
    std::sort(this->fnames.begin(), this->fnames.end());
    this->transform = transform_;
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderWithPaths} -> function{get}
// -------------------------------------------------------------------------
void datasets::ImageFolderWithPaths::get(const size_t index, std::tuple<torch::Tensor, std::string> &data){
    cv::Mat image_Mat = datasets::RGB_Loader(this->paths.at(index));
    torch::Tensor image = transforms::apply(this->transform, image_Mat);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    std::string fname = this->fnames.at(index);
    data = {image.detach().clone(), fname};
    return;
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderWithPaths} -> function{size}
// -------------------------------------------------------------------------
size_t datasets::ImageFolderWithPaths::size(){
    return this->fnames.size();
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderPairWithPaths} -> constructor
// -------------------------------------------------------------------------
datasets::ImageFolderPairWithPaths::ImageFolderPairWithPaths(const std::string root1, const std::string root2, std::vector<transforms::Compose*> &transformI_, std::vector<transforms::Compose*> &transformO_){

    fs::path ROOT;
    
    ROOT = fs::path(root1);
    for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
        if (!fs::is_directory(p)){
            std::stringstream path1, fname1;
            path1 << p.path().string();
            fname1 << p.path().filename().string();
            this->paths1.push_back(path1.str());
            this->fnames1.push_back(fname1.str());
        }
    }
    std::sort(this->paths1.begin(), this->paths1.end());
    std::sort(this->fnames1.begin(), this->fnames1.end());

    ROOT = fs::path(root2);
    for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
        if (!fs::is_directory(p)){
            std::stringstream path2, fname2;
            path2 << p.path().string();
            fname2 << p.path().filename().string();
            this->paths2.push_back(path2.str());
            this->fnames2.push_back(fname2.str());
        }
    }
    std::sort(this->paths2.begin(), this->paths2.end());
    std::sort(this->fnames2.begin(), this->fnames2.end());

    this->transformI = transformI_;
    this->transformO = transformO_;

}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderPairWithPaths} -> function{get}
// -------------------------------------------------------------------------
void datasets::ImageFolderPairWithPaths::get(const size_t index, std::tuple<torch::Tensor, torch::Tensor, std::string, std::string> &data){
    cv::Mat image_Mat1 = datasets::RGB_Loader(this->paths1.at(index));
    cv::Mat image_Mat2 = datasets::RGB_Loader(this->paths2.at(index));
    torch::Tensor image1 = transforms::apply(this->transformI, image_Mat1);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    torch::Tensor image2 = transforms::apply(this->transformO, image_Mat2);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    std::string fname1 = this->fnames1.at(index);
    std::string fname2 = this->fnames2.at(index);
    data = {image1.detach().clone(), image2.detach().clone(), fname1, fname2};
    return;
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderPairWithPaths} -> function{size}
// -------------------------------------------------------------------------
size_t datasets::ImageFolderPairWithPaths::size(){
    return this->fnames1.size();
}


// ----------------------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderPairAndRandomSamplingWithPaths} -> constructor
// ----------------------------------------------------------------------------------------
datasets::ImageFolderPairAndRandomSamplingWithPaths::ImageFolderPairAndRandomSamplingWithPaths(const std::string root1, const std::string root2, const std::string root_rand, std::vector<transforms::Compose*> &transformI_, std::vector<transforms::Compose*> &transformO_, std::vector<transforms::Compose*> &transform_rand_){

    fs::path ROOT;
    
    ROOT = fs::path(root1);
    for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
        if (!fs::is_directory(p)){
            std::stringstream path1, fname1;
            path1 << p.path().string();
            fname1 << p.path().filename().string();
            this->paths1.push_back(path1.str());
            this->fnames1.push_back(fname1.str());
        }
    }
    std::sort(this->paths1.begin(), this->paths1.end());
    std::sort(this->fnames1.begin(), this->fnames1.end());

    ROOT = fs::path(root2);
    for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
        if (!fs::is_directory(p)){
            std::stringstream path2, fname2;
            path2 << p.path().string();
            fname2 << p.path().filename().string();
            this->paths2.push_back(path2.str());
            this->fnames2.push_back(fname2.str());
        }
    }
    std::sort(this->paths2.begin(), this->paths2.end());
    std::sort(this->fnames2.begin(), this->fnames2.end());

    ROOT = fs::path(root_rand);
    for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
        if (!fs::is_directory(p)){
            std::stringstream path_rand, fname_rand;
            path_rand << p.path().string();
            fname_rand << p.path().filename().string();
            this->paths_rand.push_back(path_rand.str());
            this->fnames_rand.push_back(fname_rand.str());
        }
    }
    std::sort(this->paths_rand.begin(), this->paths_rand.end());
    std::sort(this->fnames_rand.begin(), this->fnames_rand.end());

    this->transformI = transformI_;
    this->transformO = transformO_;
    this->transform_rand = transform_rand_;

}


// ------------------------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderPairAndRandomSamplingWithPaths} -> function{get}
// ------------------------------------------------------------------------------------------
void datasets::ImageFolderPairAndRandomSamplingWithPaths::get(const size_t index, const size_t index_rand, std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, std::string, std::string, std::string> &data){
    cv::Mat image_Mat1 = datasets::RGB_Loader(this->paths1.at(index));
    cv::Mat image_Mat2 = datasets::RGB_Loader(this->paths2.at(index));
    cv::Mat image_Mat_rand = datasets::RGB_Loader(this->paths_rand.at(index_rand));
    torch::Tensor image1 = transforms::apply(this->transformI, image_Mat1);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    torch::Tensor image2 = transforms::apply(this->transformO, image_Mat2);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    torch::Tensor image_rand = transforms::apply(this->transform_rand, image_Mat_rand);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    std::string fname1 = this->fnames1.at(index);
    std::string fname2 = this->fnames2.at(index);
    std::string fname_rand = this->fnames_rand.at(index_rand);
    data = {image1.detach().clone(), image2.detach().clone(), image_rand.detach().clone(), fname1, fname2, fname_rand};
    return;
}


// -------------------------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderPairAndRandomSamplingWithPaths} -> function{size}
// -------------------------------------------------------------------------------------------
size_t datasets::ImageFolderPairAndRandomSamplingWithPaths::size(){
    return this->fnames1.size();
}


// -----------------------------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderPairAndRandomSamplingWithPaths} -> function{size_rand}
// -----------------------------------------------------------------------------------------------
size_t datasets::ImageFolderPairAndRandomSamplingWithPaths::size_rand(){
    return this->fnames_rand.size();
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderSegmentWithPaths} -> constructor
// -------------------------------------------------------------------------
datasets::ImageFolderSegmentWithPaths::ImageFolderSegmentWithPaths(const std::string root1, const std::string root2, std::vector<transforms::Compose*> &transformI_, std::vector<transforms::Compose*> &transformO_){

    fs::path ROOT = fs::path(root1);
    for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
        if (!fs::is_directory(p)){
            std::stringstream path1, fname;
            path1 << p.path().string();
            fname << p.path().filename().string();
            this->paths1.push_back(path1.str());
            this->fnames1.push_back(fname.str());
        }
    }
    std::sort(this->paths1.begin(), this->paths1.end());
    std::sort(this->fnames1.begin(), this->fnames1.end());

    std::string f_png;
    std::string::size_type pos;
    for (auto &f : this->fnames1){
        if ((pos = f.find_last_of(".")) == std::string::npos){
            f_png = f + ".png";
        }
        else{
            f_png = f.substr(0, pos) + ".png";
        }
        std::string path2 = root2 + '/' + f_png;
        this->fnames2.push_back(f_png);
        this->paths2.push_back(path2);
    }

    this->transformI = transformI_;
    this->transformO = transformO_;

    png::image<png::index_pixel> Index_png(paths2.at(0));
    png::palette pal = Index_png.get_palette();
    for (auto &p : pal){
        this->label_palette.push_back({(unsigned char)p.red, (unsigned char)p.green, (unsigned char)p.blue});
    }

}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderSegmentWithPaths} -> function{get}
// -------------------------------------------------------------------------
void datasets::ImageFolderSegmentWithPaths::get(const size_t index, std::tuple<torch::Tensor, torch::Tensor, std::string, std::string, std::vector<std::tuple<unsigned char, unsigned char, unsigned char>>> &data){
    cv::Mat image_Mat1 = datasets::RGB_Loader(this->paths1.at(index));
    cv::Mat image_Mat2 = datasets::Index_Loader(this->paths2.at(index));
    torch::Tensor image1 = transforms::apply(this->transformI, image_Mat1);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    torch::Tensor image2 = transforms::apply(this->transformO, image_Mat2);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    std::string fname1 = this->fnames1.at(index);
    std::string fname2 = this->fnames2.at(index);
    data = {image1.detach().clone(), image2.detach().clone(), fname1, fname2, this->label_palette};
    return;
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderSegmentWithPaths} -> function{size}
// -------------------------------------------------------------------------
size_t datasets::ImageFolderSegmentWithPaths::size(){
    return this->fnames1.size();
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderClassesWithPaths} -> constructor
// -------------------------------------------------------------------------
datasets::ImageFolderClassesWithPaths::ImageFolderClassesWithPaths(const std::string root, std::vector<transforms::Compose*> &transform_, const std::vector<std::string> class_names){
    std::string class_name, class_root;
    fs::path ROOT;
    for (size_t i = 0; i < class_names.size(); i++){
        std::vector<std::string> paths_tmp, fnames_tmp;
        class_name = class_names.at(i);
        class_root = root + '/' + class_name;
        ROOT = fs::path(class_root);
        for (auto &p : boost::make_iterator_range(fs::directory_iterator(ROOT), {})){
            if (!fs::is_directory(p)){
                std::stringstream rpath, fname;
                rpath << p.path().string();
                fname << p.path().filename().string();
                paths_tmp.push_back(rpath.str());
                fnames_tmp.push_back(class_name + '/' + fname.str());
                class_ids.push_back(i);
            }
        }
        std::sort(paths_tmp.begin(), paths_tmp.end());
        std::sort(fnames_tmp.begin(), fnames_tmp.end());
        std::copy(paths_tmp.begin(), paths_tmp.end(), std::back_inserter(this->paths));
        std::copy(fnames_tmp.begin(), fnames_tmp.end(), std::back_inserter(this->fnames));
    }
    this->transform = transform_;
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderClassesWithPaths} -> function{get}
// -------------------------------------------------------------------------
void datasets::ImageFolderClassesWithPaths::get(const size_t index, std::tuple<torch::Tensor,  torch::Tensor, std::string> &data){
    cv::Mat image_Mat = datasets::RGB_Loader(this->paths.at(index));
    torch::Tensor image = transforms::apply(this->transform, image_Mat);  // Mat Image ==={Resize,ToTensor,etc.}===> Tensor Image
    torch::Tensor class_id = torch::full({}, (long int)this->class_ids.at(index), torch::TensorOptions().dtype(torch::kLong));
    std::string fname = this->fnames.at(index);
    data = {image.detach().clone(), class_id.detach().clone(), fname};
    return;
}


// -------------------------------------------------------------------------
// namespace{datasets} -> class{ImageFolderClassesWithPaths} -> function{size}
// -------------------------------------------------------------------------
size_t datasets::ImageFolderClassesWithPaths::size(){
    return this->fnames.size();
}

