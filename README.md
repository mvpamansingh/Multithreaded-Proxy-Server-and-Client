#                             Multithreaded Proxy Server and Client👾

## Motivation to build this project 💡💡
- To Understand →
  - The working of requests from our local computer to the server.
  - The handling of multiple client requests from various clients.
  - Locking procedure for concurrency.
  - The concept of cache and its different functions that might be used by browsers.
- Proxy Server do →
  - It speeds up the process and reduces the traffic on the server side.
  - It can be used to restrict user from accessing specific websites.
  - A good proxy will change the IP such that the server wouldn’t know about the client who sent the request.
  - Changes can be made in Proxy to encrypt the requests, to stop anyone accessing the request illegally from your client.


## Workflow of the application 👽
![diagram-export-05-11-2024-21_48_03](https://github.com/user-attachments/assets/d86bb151-12d2-4f1a-975e-cf24d246c0bc)

## High-Level Design👾
![UML (1)](https://github.com/user-attachments/assets/382ad250-aed5-4591-bf5e-e5410b9c1e03)


## Tools and Teachnology Used🛠️
- Mutex Lock
- Semaphore
- Threading
- Cache (Time based LRU cache)


## Limitations of the project 🤖
- If a URL opens multiple clients itself, then our cache will store each client’s response as a separate element in the linked list. So, during retrieval from the cache, only a chunk of response will be send and the website will not open
- Fixed size of cache element, so big websites may not be stored in cache.


## How this project can be extended? 💡
- This code can be implemented using multiprocessing that can speed up the process with parallelism.
- We can decide which type of websites should be allowed by extending the code.
- We can implement requests like POST with this code.

## Project SetUp 📝
  - Clone the repository by the following link and open the project in CLion ide
  - > ```https://github.com/mvpamansingh/Multithreaded-Proxy-Server-and-Client.git```

  - >`make all`
 
  - >`./proxy <port no.>`


`Open http://localhost:port/https://www.cs.princeton.edu/`
## Demo 💡
![8B617ABD-206C-49F8-B52A-56A512826A00_1_201_a](https://github.com/user-attachments/assets/b7df7f4e-095d-41d7-9e7f-a27376c72e15)

## Connect with me😊
- Linkedin : https://www.linkedin.com/in/mvpamansingh/
- X : https://x.com/mvpamansingh
- mail : amansingh.as9@outlook.com
  
## Copyrights
`MIT License

Copyright (c) 2024 mvpamansingh

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.`


  
