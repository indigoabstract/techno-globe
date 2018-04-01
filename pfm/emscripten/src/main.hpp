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
   virtual int get_screen_dpi()const override;
   virtual void write_text(const char* text)const override;
   virtual void write_text_nl(const char* text)const override;
   virtual void write_text(const wchar_t* text)const override;
   virtual void write_text_nl(const wchar_t* text)const override;
   virtual void write_text_v(const char* iformat, ...)const override;
   virtual std::string get_writable_path()const override;
   umf_list get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive);
   virtual bool is_full_screen_mode();
   virtual void set_full_screen_mode(bool ienabled);
   key_types map_key(unsigned long i_key_code) const;

private:
   emst_main();
   void setup_callbacks();
   void setup_key_table();

   umf_list plist;
   bool is_started;
   static const int MAX_KEY_COUNT = 256;
   key_types key_table[MAX_KEY_COUNT];
   static shared_ptr<emst_main> instance;
};
