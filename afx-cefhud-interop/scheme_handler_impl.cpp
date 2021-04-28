// Copyright (c) 2017 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "scheme_handler_impl.h"

#include "include/cef_browser.h"
#include "include/cef_callback.h"
#include "include/cef_frame.h"
#include "include/cef_request.h"
#include "include/cef_resource_handler.h"
#include "include/cef_response.h"
#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"

#include "examples/shared/client_util.h"
#include "examples/shared/resource_util.h"

#include <stdio.h>

#include <string>
#include <map>

namespace scheme_handler {

namespace {

class CMimeType {
private:
  std::map<std::string,std::string> m_ExtToMimeType;

public:
  CMimeType() {
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
    
    m_ExtToMimeType.emplace(".aac","audio/aac");
    m_ExtToMimeType.emplace(".abw","application/x-abiword");
    m_ExtToMimeType.emplace(".arc","application/x-freearc");
    m_ExtToMimeType.emplace(".avi","video/x-msvideo");
    m_ExtToMimeType.emplace(".azw","application/vnd.amazon.ebook");
    m_ExtToMimeType.emplace(".bin","application/octet-stream");
    m_ExtToMimeType.emplace(".bmp","image/bmp");
    m_ExtToMimeType.emplace(".bz","application/x-bzip");
    m_ExtToMimeType.emplace(".bz2","application/x-bzip2");
    m_ExtToMimeType.emplace(".csh","application/x-csh");
    m_ExtToMimeType.emplace(".css","text/css");
    m_ExtToMimeType.emplace(".csv","text/csv");
    m_ExtToMimeType.emplace(".doc","application/msword");
    m_ExtToMimeType.emplace(".docx","application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    m_ExtToMimeType.emplace(".eot","application/vnd.ms-fontobject");
    m_ExtToMimeType.emplace(".epub","application/epub+zip");
    m_ExtToMimeType.emplace(".gz","application/gzip");
    m_ExtToMimeType.emplace(".gif","image/gif");
    m_ExtToMimeType.emplace(".htm","text/html");
    m_ExtToMimeType.emplace(".html","text/html");
    m_ExtToMimeType.emplace(".ico","image/vnd.microsoft.icon");
    m_ExtToMimeType.emplace(".ics","text/calendar");
    m_ExtToMimeType.emplace(".jar","application/java-archive");
    m_ExtToMimeType.emplace(".jpeg","image/jpeg");
    m_ExtToMimeType.emplace(".jpg","image/jpeg");
    m_ExtToMimeType.emplace(".js","text/javascript");
    m_ExtToMimeType.emplace(".json","application/json");
    m_ExtToMimeType.emplace(".jsonld","application/ld+json");
    m_ExtToMimeType.emplace(".mid","audio/midi");
    m_ExtToMimeType.emplace(".midi","audio/midi");
    m_ExtToMimeType.emplace(".mjs","text/javascript");
    m_ExtToMimeType.emplace(".mp3","audio/mpeg");
    m_ExtToMimeType.emplace(".cda","application/x-cdf");
    m_ExtToMimeType.emplace(".mp4","video/mp4");
    m_ExtToMimeType.emplace(".mpeg","video/mpeg");
    m_ExtToMimeType.emplace(".mpkg","application/vnd.apple.installer+xml");
    m_ExtToMimeType.emplace(".odp","application/vnd.oasis.opendocument.presentation");
    m_ExtToMimeType.emplace(".ods","application/vnd.oasis.opendocument.spreadsheet");
    m_ExtToMimeType.emplace(".odt","application/vnd.oasis.opendocument.text");
    m_ExtToMimeType.emplace(".oga","audio/ogg");
    m_ExtToMimeType.emplace(".ogv","video/ogg");
    m_ExtToMimeType.emplace(".ogx","application/ogg");
    m_ExtToMimeType.emplace(".opus","audio/opus");
    m_ExtToMimeType.emplace(".otf","font/otf");
    m_ExtToMimeType.emplace(".png","image/png");
    m_ExtToMimeType.emplace(".pdf","application/pdf");
    m_ExtToMimeType.emplace(".php","application/x-httpd-php");
    m_ExtToMimeType.emplace(".ppt","application/vnd.ms-powerpoint");
    m_ExtToMimeType.emplace(".pptx","application/vnd.openxmlformats-officedocument.presentationml.presentation");
    m_ExtToMimeType.emplace(".rar","application/vnd.rar");
    m_ExtToMimeType.emplace(".rtf","application/rtf");
    m_ExtToMimeType.emplace(".sh","application/x-sh");
    m_ExtToMimeType.emplace(".svg","image/svg+xml");
    m_ExtToMimeType.emplace(".swf","application/x-shockwave-flash");
    m_ExtToMimeType.emplace(".tar","application/x-tar");
    m_ExtToMimeType.emplace(".tif","image/tiff");
    m_ExtToMimeType.emplace(".tiff","image/tiff");
    m_ExtToMimeType.emplace(".ts","video/mp2t");
    m_ExtToMimeType.emplace(".ttf","font/ttf");
    m_ExtToMimeType.emplace(".txt","text/plain");
    m_ExtToMimeType.emplace(".vsd","application/vnd.visio");
    m_ExtToMimeType.emplace(".wav","audio/wav");
    m_ExtToMimeType.emplace(".weba","audio/webm");
    m_ExtToMimeType.emplace(".webm","video/webm");
    m_ExtToMimeType.emplace(".webp","image/webp");
    m_ExtToMimeType.emplace(".woff","font/woff");
    m_ExtToMimeType.emplace(".woff2","font/woff2");
    m_ExtToMimeType.emplace(".xhtml","application/xhtml+xml");
    m_ExtToMimeType.emplace(".xls","application/vnd.ms-excel");
    m_ExtToMimeType.emplace(".xlsx","application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    m_ExtToMimeType.emplace(".xml","application/xml");
    m_ExtToMimeType.emplace(".xul","application/vnd.mozilla.xul+xml");
    m_ExtToMimeType.emplace(".zip","application/zip");
    m_ExtToMimeType.emplace(".3gp","video/3gpp");
    m_ExtToMimeType.emplace(".3g2","video/3gpp2");
    m_ExtToMimeType.emplace(".7z","application/x-7z-compressed");    
  }

  std::string Get(const std::string & url) {
    size_t pos = url.rfind(".");
    if(pos != std::string::npos)
    {
      auto it = m_ExtToMimeType.find(url.substr(pos));
      if(it != m_ExtToMimeType.end()) return it->second;
    }

    return "application/octet-stream";
  }

} g_MimeTypes;

bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// Implementation of the scheme handler for client:// requests.
class ClientSchemeHandler : public CefResourceHandler {
 public:
  ClientSchemeHandler() : offset_(0) {}

  ~ClientSchemeHandler() {
    if(file_ != NULL) fclose(file_);
  }

  bool ProcessRequest(CefRefPtr<CefRequest> request,
                      CefRefPtr<CefCallback> callback) OVERRIDE {
    CEF_REQUIRE_IO_THREAD();
    
    std::string url = request->GetURL();

    std::string prefix = "afx://file/";

    if(0 == url.find(prefix))
    {
      url = url.substr(prefix.length());

      size_t pos_query = url.find("?");
      if(std::string::npos != pos_query)
      {
        url = url.substr(0, pos_query);
      }

      size_t pos_hash = url.find("#"); 
      if(std::string::npos != pos_hash)
      {
        url = url.substr(0, pos_hash);
      }    

      if(file_ != NULL) fclose(file_);
      file_ = fopen(url.c_str(), "rb");
      if(NULL != file_)
      {
        mime_type_ = g_MimeTypes.Get(url);

        fseek(file_, 0, SEEK_END);
        file_size_ =  (int64)ftell(file_);
        fseek(file_, 0, SEEK_SET);

        callback->Continue();
        return true;
      }
    }

    return false;
  }

  void GetResponseHeaders(CefRefPtr<CefResponse> response,
                          int64& response_length,
                          CefString& redirectUrl) OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    DCHECK(NULL != file_);

    response->SetMimeType(mime_type_);
    response->SetStatus(200);

    // Set the resulting response length.
    response_length = file_size_;
  }

  void Cancel() OVERRIDE { CEF_REQUIRE_IO_THREAD(); }

  bool ReadResponse(void* data_out,
                    int bytes_to_read,
                    int& bytes_read,
                    CefRefPtr<CefCallback> callback) OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    bool has_data = false;
    bytes_read = 0;

    if (offset_ < file_size_) {
      // Copy the next block of data into the buffer.
      int transfer_size =
          std::min(bytes_to_read, static_cast<int>(file_size_ - offset_));
      bytes_read = (int)fread(data_out,1,transfer_size,file_);
      offset_ += bytes_read;
      
      has_data = true;
    }

    return has_data;
  }

 private:
  FILE * file_ = NULL;
  std::string mime_type_;
  size_t offset_;
  long file_size_;

  IMPLEMENT_REFCOUNTING(ClientSchemeHandler);
  DISALLOW_COPY_AND_ASSIGN(ClientSchemeHandler);
};

// Implementation of the factory for creating scheme handlers.
class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory {
 public:
  ClientSchemeHandlerFactory() {}

  // Return a new scheme handler instance to handle the request.
  CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       const CefString& scheme_name,
                                       CefRefPtr<CefRequest> request) OVERRIDE {
    CEF_REQUIRE_IO_THREAD();
   
    return new ClientSchemeHandler();
  }

 private:
  bool enabled_;

  IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
  DISALLOW_COPY_AND_ASSIGN(ClientSchemeHandlerFactory);
};

}  // namespace

void RegisterSchemeHandlerFactory() {
  CefRegisterSchemeHandlerFactory("afx", "file",
                                  new ClientSchemeHandlerFactory());
}

}  // namespace scheme_handler
