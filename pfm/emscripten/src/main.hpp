#pragma once

#include "pfm.hpp"


class emst_main : public pfm_main
{
public:
   virtual ~emst_main();
   static shared_ptr<emst_main> get_instance();
   static shared_ptr<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir);
   virtual void init() override;
   virtual void start() override;
   virtual void run() override;
   virtual int get_screen_dpi()const;
   virtual void write_text(const char* text)const;
   virtual void write_text_nl(const char* text)const;
   virtual void write_text(const wchar_t* text)const;
   virtual void write_text_nl(const wchar_t* text)const;
   virtual void write_text_v(const char* iformat, ...)const;
   virtual std::string get_writable_path()const;
   umf_list get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive);
   virtual bool is_full_screen_mode();
   virtual void set_full_screen_mode(bool ienabled);

private:
   emst_main();
   void setup_callbacks();

   umf_list plist;
   bool is_started;
   static shared_ptr<emst_main> instance;
};
