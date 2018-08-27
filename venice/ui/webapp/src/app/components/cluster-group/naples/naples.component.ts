import { Component, OnInit, ViewEncapsulation, ViewChild, OnDestroy } from '@angular/core';
import { Table } from 'primeng/table';
import { IClusterSmartNIC, ClusterSmartNICSpec, IApiStatus } from '@sdk/v1/models/generated/cluster';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Subscription } from 'rxjs/Subscription';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-naples',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './naples.component.html',
  styleUrls: ['./naples.component.scss']
})
export class NaplesComponent implements OnInit, OnDestroy {
  @ViewChild('naplesTable') naplesTurboTable: Table;

  naples: ReadonlyArray<IClusterSmartNIC> = [];
  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility;

  cols: any[] = [
    { field: 'meta.name', header: 'Name', class: 'naples-column-date', sortable: false },
    { field: 'spec.host-name', header: 'Host name', class: 'naples-column-host-name', sortable: false },
    { field: 'spec.mgmt-ip', header: 'Management IP', class: 'naples-column-mgmt-ip', sortable: false },
    { field: 'spec.phase', header: 'Phase', class: 'naples-column-phase', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'naples-column-date', sortable: false },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'naples-column-date', sortable: false },
  ];
  subscriptions: Subscription[] = [];

  phaseUIHint = ClusterSmartNICSpec.propInfo.phase.enum;

  naplesIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    svgIcon: 'naples'
  };

  constructor(private clusterService: ClusterService,
    protected controllerService: ControllerService,
  ) { }

  ngOnInit() {
    this.getNaples();

    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Cluster', url: '' }, { label: 'Naples', url: '' }]
    });
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility();
    this.naples = this.naplesEventUtility.array as ReadonlyArray<IClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
      response => {
        const body: any = response.body;
        this.naplesEventUtility.processEvents(body);
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.log('Cluster service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    )
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

}
