import { Component, OnInit } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-missingpage',
  templateUrl: './missingpage.component.html',
  styleUrls: ['./missingpage.component.scss']
})
export class MissingpageComponent implements OnInit {

  constructor(protected controllerService: ControllerService) { }

  ngOnInit() {
    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: []
    });
  }

}
