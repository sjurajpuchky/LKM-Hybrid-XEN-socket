# LKM-Hybrid-XEN-socket

> Provide unix socket over XEN Socket implementation as interconnection on XEN VM environment for related services and api over unix socket, 
> benefit is that is not needed to implement client/server as part of connection to services for high performance.

## Inspiration from projects
> Based on research https://www.researchgate.net/publication/221461399_XenSocket_A_High-Throughput_Interdomain_Transport_for_Virtual_Machines
> Inspired by implementation https://github.com/skranjac/XVMSocket


## Compile

## Setup

## Security

## Performance

## Samples

### Database VM interconnection
### PHP-FPM VM interconnection
### CrossOver Nginx ByPass Proxy balancer

```
b|------------|
a| endpoint 1 |-|-HXS-|----|
l|------------| |     |  f |-- MEM DB      -HXS-|
a               |     | cgi|-- MEM STORAGE -HXS-- PERSISTENT STORAGE
n|------------| |     |    |
c| endpoint 2 |-|-HXS-|----|
e|------------| |
r               |
          |-----------|
 ---------|    CDN    |
          |-----------|      
```
###