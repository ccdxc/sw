 window.setTimeout (function(e) { 
      try {
        var list = document.getElementsByClassName("app-loading");
        if (list && list.length > 0) {
          alert("Failed to load Venice UI, please clear cache and reload page or contact administrator.");
        }
      } catch (error) {
        console.error('Venice-UI index.html and index.js ', error);
      }
      // index.html links to this js file. When browser window starts, it checks whether the loading indicator is still there. If so, Venice has problem either due to backend or due to certificate issue
      
 }, 60000);
