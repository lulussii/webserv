# Webserv

*This project has been created as part of the 42 curriculum by mlaussel and lserodon*

## Description
The webserv project consists of coding an HTTP server in C++. It is based on a non-blocking architecture that can handle several simultaneous connections.
The server processes GET, POST, and DELETE methods, allows the distribution of static files, and supports the execution of dynamic scripts in PHP thanks to CGI.

## Features 

* HTTP/1.1 Protocol : Implementation of a functional HTTP server compliant with basic RFC standards.
* I/O Multiplexing: Non-blocking event loop using poll() to handle multiple simultaneous clients.
* Methods: Support for GET, POST, and DELETE requests.
* Static File Serving: Efficient delivery of HTML, CSS, images, and other static assets.
* CGI Support: Execution of dynamic scripts in PHP via the Common Gateway Interface.
* Directory Listing: Automatic generation of an "autoindex" page when no index file is present.
* Custom Error Pages: Configurable HTML pages for specific HTTP error codes (e.g., 404, 405, 500).
* File Upload: Support for uploading files to the server via POST requests.

## Instructions

### Compilation and Run

```bash
make
```
The server can be launched with a custom configuration file, added as an argument. If no configuration file is provided as an argument, a default file will be used.

```bash
./webserv config_file.conf
```
The configuration file format is inspired by the nginx syntax:

```bash
server 
{
    listen 8080;                               # listen port
    root ./www;                                # Root folder for files
    index index.html;                          # Default file
    client_max_body_size 1000000;              # Size limit for Uploads

    # Custom error management
    error_page 404  /html/404.html;

    # Route for static files
    location / 
    {
        methods GET;
        autoindex on;                          # Enables directory listing
    }

    # Specific route for Upload
    location /upload 
    {
        methods POST DELETE;
        upload_dir ./www/uploads;              # Destination folder for files
    }

    # Dynamic content configuration (CGI)
    location /cgi-bin 
    {
        methods GET POST;
        cgi_setup php .php /usr/bin/php-cgi;   # CGI 
    }
}
```

### How to use siege

```bash
siege -c50 -t30S http://localhost:8080/
```


## Resources

[HTTP Server: Everything you need to know to Build a simple HTTP server from scratch](https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)

[HTTP: Hypertext Transfer Protocol](https://developer.mozilla.org/en-US/docs/Web/HTTP)

[Webserv by achrafelkhnissi](https://github.com/achrafelkhnissi/webserv?tab=readme-ov-file)

[La documentation de webserv](https://hackmd.io/@fttranscendance/H1mLWxbr_)

[Hypertext Transfer Protocol Wikipedia Page](https://fr.wikipedia.org/wiki/Hypertext_Transfer_Protocol)

[Status Code](https://www.ibm.com/docs/fr/power11/9080-HEU?topic=protocol-http-status-codes)

[Status Code](https://developer.mozilla.org/fr/docs/Web/HTTP/Reference/Status)

### IA Disclosure

Artificial Intelligence was used as a pedagogical assistant during this project. It helped in understanding the theoretical aspects of the HTTP protocol, as well as assisting with debugging and documentation structuring. However, all the core logic, server architecture, and memory management were entirely designed and implemented by the team.
