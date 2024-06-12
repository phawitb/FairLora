var ss = SpreadsheetApp.openById('1TSYMyFLOWLXsRZe5JBBe1sX1zNaMwaYvPRZMfQqlRYg')
var sheet_last = ss.getSheetByName('last')
var sheet_config = ss.getSheetByName('config')
var MAX_ROW = 300000

function findMaxIndex(sheet,columnRange) {
  var max = null;
  var maxIndex = 0;
  // var sheet = SpreadsheetApp.getActiveSheet();
  var data = sheet.getRange(columnRange).getValues();

  for (var i = 0; i < data.length; i++) {
    var currentValue = data[i][0]; // Assuming single-column data
    if (currentValue > max) {
      max = currentValue;
      maxIndex = i + 1; // Adjust for 1-based indexing
    }
    else if(max === null){
      maxIndex = 0
    }
  }
  return maxIndex;
}
function updateRowByIndex(sheet_id,rowIndex, data) {
  // var sheet = SpreadsheetApp.getActiveSheet();
  
  var range = sheet_id.getRange(rowIndex, 1, 1, data.length); // Adjust range based on data length
  range.setValues([data]);
}
function findValueIndex(sheet, columnRange, value) {
  var data = sheet.getRange(columnRange).getValues();
  for (var i = 0; i < data.length; i++) {
    if (data[i][0] === value) { // Assuming single-column data
      return i + 1; // Adjust for 1-based indexing
    }
  }
  return -1; // Not found
}

// get data --------------------------------------------------------------------------------

function doGet(e) {
  var id = e.parameter.id
  var action = e.parameter.action

  if (action == 'getConfig') {
      return getConfig(id)
    }
  if (action == 'getLast') {
      return getLast(id)
    }
  if (action == 'getDatas') {
      return getDatas(id)
    }
}

function getLast(id) {
  var rows = sheet_last.getRange(2,1,sheet_last.getLastRow()-1,sheet_last.getLastColumn()).getValues()
  var data = {}

  
  for(var i=0; i<rows.length;i++){
    var row = rows[i]
    if(row[0] == id){
      // var record = {}
      data['id']=row[0]
      data['date']=row[1]
      data['temp']=row[2]
      data['humid']=row[3]
      data['hic']=row[4]
      data['co2']=row[5]
      data['flag']=row[6]
     

    }

  }
  var result = JSON.stringify(data)
  return ContentService.createTextOutput(result).setMimeType(ContentService.MimeType.JSON)
}

function getConfig(id) {
  var rows = sheet_config.getRange(2,1,sheet_config.getLastRow()-1,sheet_config.getLastColumn()).getValues()
  var data = {}

  
  for(var i=0; i<rows.length;i++){
    var row = rows[i]
    if(row[0] == id){
      // var record = {}
      data['id']=row[0]
      data['unit']=row[1]
      data['adj_temp']=row[2]
      data['adj_humid']=row[3]
      data['adj_co2']=row[4]
      data['line1']=row[5]
      data['line2']=row[6]
      data['line3']=row[7]

    }

  }
  var result = JSON.stringify(data)
  return ContentService.createTextOutput(result).setMimeType(ContentService.MimeType.JSON)
}

function getDatas(id) {
  // var user = JSON.parse(e.postData.contents)
  sheet_data = ss.getSheetByName(id)
  var rows = sheet_data.getRange(2,1,sheet_data.getLastRow()-1,sheet_data.getLastColumn()).getValues()
  var data =[]
  for(var i=0; i<rows.length;i++){
    var row = rows[i]
    var record = {}
        record['date']=row[0]
        record['id']=row[1]
        record['temp']=row[2]
        record['humid']=row[3]
        record['hic']=row[4]
        record['co2']=row[5]
        record['flag']=row[6]
        data.push(record)
  }
  var result = JSON.stringify(data)
  return ContentService.createTextOutput(result).setMimeType(ContentService.MimeType.JSON)
}

// add data --------------------------------------------------------------------------------
function doPost(e) {
  var action = e.parameter.action
  if (action == 'addConfig') {
    return addConfig(e)
  }
  if (action == 'addData') {
    return addData(e)
  }
}

function addData(e) {
  var user = JSON.parse(e.postData.contents)
  var sheet_id = ss.getSheetByName(user.id)
  var rowIndex = findMaxIndex(sheet_id,`A2:A${MAX_ROW}`)+2
  if(rowIndex%(MAX_ROW+1)==0){
    rowIndex += 2
  }
  if(rowIndex==0){
    rowIndex = 2
  }

  // var idx = findValueIndex(sheet_last, "A1:A10000", user.id)
  var idx = findValueIndex(sheet_last, `A1:A${MAX_ROW}`, user.id)
  
  if(idx!=-1){
    updateRowByIndex(sheet_last,idx, [user.id, user.date, user.temp, user.humid, user.hic, user.co2, user.flag])
  }
  
  updateRowByIndex(sheet_id,rowIndex%(MAX_ROW+1), [user.date, user.id, user.temp, user.humid, user.hic, user.co2, user.flag])
  // sheet_data.appendRow([m, "zzzz", user.temp, user.humid, user.hic, user.co2])
  // sheet_data.appendRow([user.date, user.id, user.temp, user.humid, user.hic, user.co2])
  return ContentService.createTextOutput("success").setMimeType(ContentService.MimeType.TEXT)
}

function addConfig(e) {
  var user = JSON.parse(e.postData.contents)
  var idx = findValueIndex(sheet_config, `A1:A${MAX_ROW}`, user.id)

  updateRowByIndex(sheet_config,idx, [user.id,user.unit,user.adj_temp,user.adj_humid,user.adj_co2,user.line1,user.line2,user.line3])
  
  return ContentService.createTextOutput("success").setMimeType(ContentService.MimeType.TEXT)
}





// var ss = SpreadsheetApp.openById('1vF4hWOl_bB6EnGmqNFT0iyfbQhS_COGW9tIAcVlVU6o')
// var sheet = ss.getSheetByName('setup')



// var ss = SpreadsheetApp.openById('xxx')
// var sheet = ss.getSheetByName('xxx')



// date id  temp  humid hic co2

// function addUser(e) {
//   var user = JSON.parse(e.postData.contents)
//   sheet_setup.appendRow([user.id, user.unit, user.adj_temp, user.adj_humid, user.adj_co2, user.line1, user.line2, user.line3])
//   return ContentService.createTextOutput("success").setMimeType(ContentService.MimeType.TEXT)
// }

// date id  temp  humid hic co2
