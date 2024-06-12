# FairLora
#### App
https://iotlora.streamlit.app/

#### Sheet
https://docs.google.com/spreadsheets/d/1TSYMyFLOWLXsRZe5JBBe1sX1zNaMwaYvPRZMfQqlRYg/edit?gid=1213476532#gid=1213476532  
#### Setup API
https://www.youtube.com/watch?reload=9&v=r817RLqmLac

### API test
```
post data : https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=addData

{
    "date":91715193007,
    "id" : "F001",
    "temp" : 12.33,
    "humid" : 43.3,
    "hic" : 60.78,
    "co2" : 123.2,
    "flag" : "M001"

}
```
```
get last : https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?id=F001&action=getLast
```
```
get config : https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=getConfig&id=F001
```
```
update config : https://script.google.com/macros/s/AKfycbz7gxBfH1DUkWCXwtYFFTNVkNschcJfjCdh8WyXNjvKmOfUefk4Hwf3UxeEy5Y8S8ffrQ/exec?action=addConfig

{
"id":"F001",
"unit":"F1",
"adj_temp":0,
"adj_humid":0,
"adj_co2":0,
"line1":"kuYW00r39JrvNhju1S8WPImarC3uz9qUNXlo784KjJg",
"line2":"",
"line3":""
}
```


