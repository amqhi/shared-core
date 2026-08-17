#ifndef SHARED_MIME_UTILS_H
#define SHARED_MIME_UTILS_H

#pragma once

// Based on Chromium's mime_util

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>

namespace mime_util {

namespace detail {

struct MimeInfo {
    std::string_view mime_type;
    std::string_view extensions; // comma-separated, first is preferred
};

// From Chromium net/base/mime_util.cc - kPrimaryMappings
// These take priority over platform / kSecondaryMappings
static constexpr MimeInfo primary_mappings[] = {
    // Must precede audio/webm
    {"video/webm",          "webm"},
    // Must precede audio/mp3
    {"audio/mpeg",          "mp3"},

    {"application/wasm",        "wasm"},
    {"application/xhtml+xml",   "xhtml,xht,xhtm"},
    {"audio/flac",              "flac"},
    {"audio/matroska",          "mka"},
    {"audio/mp3",               "mp3"},
    {"audio/ogg",               "ogg,oga,opus"},
    {"audio/wav",               "wav"},
    {"audio/webm",              "webm"},
    {"audio/x-m4a",             "m4a"},
    {"image/avif",              "avif"},
    {"image/gif",               "gif"},
    {"image/jpeg",              "jpeg,jpg,jpe"},
    {"image/jxl",               "jxl"},
    {"image/png",               "png"},
    {"image/apng",              "png,apng"},
    {"image/svg+xml",           "svg,svgz"},
    {"image/webp",              "webp"},
    {"multipart/related",       "mht,mhtml"},
    {"text/css",                "css"},
    {"text/html",               "html,htm,shtml,shtm"},
    {"text/javascript",         "js,mjs"},
    {"text/xml",                "xml"},
    {"video/matroska",          "mkv"},
    {"video/mp4",               "mp4,m4v"},
    {"video/ogg",               "ogv,ogm"},
    // Overrides Windows Excel registration of csv
    {"text/csv",                "csv"},
};

// From Chromium net/base/mime_util.cc - kSecondaryMappings
// Can be overridden by platform
static constexpr MimeInfo secondary_mappings[] = {
    // Must precede image/vnd.microsoft.icon
    {"image/x-icon",            "ico"},

    {"application/epub+zip",    "epub"},
    {"application/font-woff",   "woff"},
    {"application/gzip",        "gz,tgz"},
    {"application/javascript",  "js"},
    {"application/json",        "json"},
    {"application/msword",      "doc,dot"},
    {"application/octet-stream","bin,exe,com"},
    {"application/pdf",         "pdf"},
    {"application/postscript",  "ps,eps,ai"},
    {"application/rdf+xml",     "rdf"},
    {"application/rss+xml",     "rss"},
    {"application/rtf",         "rtf"},
    {"application/vnd.android.package-archive", "apk"},
    {"application/vnd.ms-excel","xls"},
    {"application/vnd.ms-powerpoint", "ppt"},
    {"application/vnd.openxmlformats-officedocument.presentationml.presentation", "pptx"},
    {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "xlsx"},
    {"application/vnd.openxmlformats-officedocument.wordprocessingml.document", "docx"},
    {"application/x-gzip",      "gz,tgz"},
    {"application/x-mpegurl",   "m3u8"},
    {"application/x-tar",       "tar"},
    {"application/x-x509-ca-cert", "cer,crt"},
    {"application/zip",         "zip"},
    {"audio/webm",              "weba"},
    {"image/bmp",               "bmp"},
    {"image/jpeg",              "jfif,pjpeg,pjp"},
    {"image/tiff",              "tiff,tif"},
    {"image/vnd.microsoft.icon","ico"},
    {"image/x-png",             "png"},
    {"image/x-xbitmap",         "xbm"},
    {"message/rfc822",          "eml"},
    {"text/calendar",           "ics"},
    {"text/html",               "ehtml"},
    {"text/markdown",           "md"},
    {"text/plain",              "txt,text"},
    {"text/vtt",                "vtt"},
    {"text/x-sh",               "sh"},
    {"text/xml",                "xsl,xbl,xslt"},
    {"video/mpeg",              "mpeg,mpg,mpe"},
};

inline bool equals_case_insensitive(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    return std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
        return std::tolower((unsigned char)x) == std::tolower((unsigned char)y);
    });
}

// Searches mappings for the given extension (case-insensitive).
// Returns the mime type if found.
template <size_t N>
std::optional<std::string_view> FindMimeType(
    const MimeInfo (&mappings)[N],
    std::string_view ext)
{
    for (const auto& mapping : mappings) {
        std::string_view exts = mapping.extensions;
        while (!exts.empty()) {
            std::string_view token;
            auto comma = exts.find(',');
            if (comma == std::string_view::npos) {
                token = exts;
                exts  = {};
            } else {
                token = exts.substr(0, comma);
                exts  = exts.substr(comma + 1);
            }
            // trim spaces
            while (!token.empty() && token.front() == ' ') token.remove_prefix(1);
            while (!token.empty() && token.back()  == ' ') token.remove_suffix(1);

            if (equals_case_insensitive(token, ext))
                return mapping.mime_type;
        }
    }
    return std::nullopt;
}

// Extracts the extension from a file path (without leading dot, lowercased).
inline std::string extract_extension(std::string_view path) {
    // Find the last dot after the last separator
    auto sep = path.find_last_of("/\\");
    std::string_view name = (sep == std::string_view::npos) ? path : path.substr(sep + 1);

    auto dot = name.rfind('.');
    if (dot == std::string_view::npos || dot == 0)
        return {};

    std::string ext(name.substr(dot + 1));
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ext;
}

} // namespace detail

/// Returns the MIME type for the given file path based on its extension.
/// Falls back to "application/octet-stream" if the extension is unknown.
/// Pass include_path=true (default) to pass a full path; false for just an extension.
inline std::string get_mime_type_from_file(std::string_view file_path) {
    std::string ext = detail::extract_extension(file_path);
    if (ext.empty())
        return "application/octet-stream";

    if (auto m = detail::FindMimeType(detail::primary_mappings, ext))
        return std::string(*m);
    if (auto m = detail::FindMimeType(detail::secondary_mappings, ext))
        return std::string(*m);

    return "application/octet-stream";
}

/// Returns the MIME type for a bare extension string (e.g. "jpg", "html").
inline std::string get_mime_type_from_extension(std::string_view ext) {
    std::string lower(ext);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (auto m = detail::FindMimeType(detail::primary_mappings, lower))
        return std::string(*m);
    if (auto m = detail::FindMimeType(detail::secondary_mappings, lower))
        return std::string(*m);

    return "application/octet-stream";
}

} // namespace mime_util

#endif //SHARED_MIME_UTILS_H
