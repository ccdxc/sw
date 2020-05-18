import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-api-sample',
  templateUrl: './api-sample.component.html',
  styleUrls: ['./api-sample.component.scss'],
  encapsulation: ViewEncapsulation.None,
})
export class ApiSampleComponent  implements OnInit {
  APICAPTURE: string = 'apiCapture';

  enableLiveCapture: boolean = false;

  apiLiveTrack: boolean;
  bodyicon: Icon = {
    margin: {
      top: '9px',
      left: '8px',
    },
    matIcon: 'library_books'
  };
  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService ) {
      this.enableLiveCapture = (this.uiconfigsService.isFeatureEnabled(this.APICAPTURE));
    }

  ngOnInit() {

    this.setDefaultToolbar();
  }
  selectedIndexChangeEvent(event) {
    this.apiLiveTrack = event;
  }

  private setDefaultToolbar() {
    this.controllerService.setToolbarData({
      breadcrumb: [{ label: 'API Capture', url: Utility.getBaseUIUrl() + 'admin/api' }],
      buttons: [{
        cssClass: 'global-button-primary api-samples-button',
        text: (!this.enableLiveCapture) ? 'Enable Live Capture' : 'Disable Live Capture',
        callback: () => this.switchOnOffLiveCapture(),
        genTooltip: () => this.getTooltip()
      }]
    });
  }

  getTooltip(): string {
    return (!this.enableLiveCapture) ? 'Start capturing API request and response.' : 'Stop capturing API request and response.';
  }

  switchOnOffLiveCapture() {
    this.enableLiveCapture = !this.enableLiveCapture;
    this.uiconfigsService.setOneFeature('apiCapture', this.enableLiveCapture );
    this.setDefaultToolbar();
    const msg = 'Live API capture is ' + ((this.enableLiveCapture) ?  ' enabled.' : 'disabled.');
    this.controllerService.invokeInfoToaster('Info', msg);
}
}
