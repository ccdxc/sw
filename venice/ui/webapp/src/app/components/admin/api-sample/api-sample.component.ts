import { Component } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-api-sample',
  templateUrl: './api-sample.component.html',
  styleUrls: ['./api-sample.component.scss']
})
export class ApiSampleComponent {

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

  constructor() { }

  selectedIndexChangeEvent(event) {
    this.apiLiveTrack = event;
  }
}
