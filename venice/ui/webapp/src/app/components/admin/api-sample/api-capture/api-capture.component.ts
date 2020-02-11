import { Component, OnInit, Input, OnChanges } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { Utility, RestAPIRequestResponse, UrlData } from '@app/common/Utility';
import { TableCol } from '@app/components/shared/tableviewedit';
import { TableMenuItem } from '@app/components/shared/tableheader/tableheader.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Eventtypes } from '@app/enum/eventtypes.enum.js';
import * as apiStub from '../../../../../assets/apiSample.json';

@Component({
  selector: 'app-api-capture',
  templateUrl: './api-capture.component.html',
  styleUrls: ['./api-capture.component.scss']
})
export class ApiCaptureComponent implements OnInit, OnChanges {

  @Input() isLiveApiTabSelect: boolean;
  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols: TableCol[] = [
    { field: 'category', header: 'Category', width: '15%' },
    { field: 'kind', header: 'Kind', width: '20%' },
    { field: 'method', header: 'Method', width: '10%' },
    { field: 'url', header: 'URL', width: '55%'}
  ];

  tableMenuItems: TableMenuItem[] = [
    {
      text: 'Export File',
      onClick: () => {
        this.exportTableDataJSON();
      }
    }
  ];
  apiDetails: ApiData[];
  headerList: TableCol[];

  constructor(protected controllerService: ControllerService) { }

  ngOnChanges() {
    if (this.isLiveApiTabSelect) {
      this.prepareLiveApiSampleData();
    } else {
      this.prepareApiSampleData();
    }
  }

  ngOnInit() {

    this.controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'ApiSampleComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.controllerService.setToolbarData({
      breadcrumb: [{ label: 'API Capture', url: Utility.getBaseUIUrl() + 'admin/api' }]
    });

    this.headerList = (this.cols).filter((el) => {
      return el.field;
    });
  }


  prepareApiSampleData() {
    this.apiDetails = [];
    const samples: ApiData = apiStub['apiDetails'];
    let index: number = 0;
    for (const key in samples) {
      if (key) {
        const api = samples[key];
        for (const method in samples[key]) {
          if (api[method] instanceof Array) {
            index = this.addCommentsColumn(api[method] as UrlData[], index);
          } else {
            api[method].id = index; // adding index as unique id for row expand
            this.apiDetails.push(api[method] as ApiData);
            index++;
          }
        }
      }
    }
  }

  addCommentsColumn(data: UrlData[], index: number) {
    for (const comment in data) {
      if (comment) {
        data[comment].id = index;
        this.apiDetails.push(data[comment] as ApiData);
        index++;
      }
    }
    return index;
  }

  prepareLiveApiSampleData() {
    this.apiDetails = [];
    let index: number = 0;
    const samples: RestAPIRequestResponse = Utility.getInstance().veniceAPISampleMap;
    for (const key in samples) {
      if (samples.hasOwnProperty(key)) {
        const captured: UrlData = samples[key];
        captured['id'] = index;  // adding id for row expand
        this.apiDetails.push(captured as ApiData);
        index++;
      }
    }
  }

  downloadSelectedRow(selectedRow) {
    const isSearchURL: boolean  = (selectedRow.url.indexOf('/search/v1/query') >= 0 );
    const fileName: string = ((this.isLiveApiTabSelect) ? 'venice-live-api-captures_' : 'venice-api-samples_') + `${(selectedRow.category).toLowerCase()}-${(selectedRow.kind).toLowerCase()}-${(selectedRow.method).toLowerCase()}` + (isSearchURL ?  '_' + selectedRow['id'] : '');
    this.exportToJson(selectedRow, fileName);
  }

  exportTableDataJSON() {
    // Based on the tab index, we generate file name
    const actualData = this.replaceLastColumn(JSON.parse(JSON.stringify(this.apiDetails)));
    const exportFilename: string = (this.isLiveApiTabSelect) ? 'venice-live-api-captures' : 'venice-api-samples';
    this.exportToJson(actualData, exportFilename);
  }

  exportToJson(dataToBeExport: ApiData[], fileName: string) {
    const fieldName = fileName + '.json';
    Utility.exportContent(JSON.stringify(dataToBeExport, null, 2), 'text/json;charset=utf-8;', fieldName);
    Utility.getInstance().getControllerService().invokeInfoToaster('Data exported', 'Please find ' + fieldName + ' in your download folder');

  }

  convertJsonString(jsonString: string) {
    return (jsonString ? (JSON.stringify(jsonString, null, 2)) : '');
  }

  // remove the id during the data export.
  replaceLastColumn(apiData) {
    for (const key in apiData) {
      if (key) {
        delete apiData[key]['id'];
      }
    }
    return apiData;
  }
}

export interface ApiData {
  [key: string]: RestAPIRequestResponse;
}
