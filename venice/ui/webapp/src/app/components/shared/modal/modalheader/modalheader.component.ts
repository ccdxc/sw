import { Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';

/**
 * This component creates the header and takes an icon object that contains either the url
 * to use through svgIcon, or a name to use for material icons, a title, subtitle.
 * It also has an output for when user clicks the cancel button in the top right.
 * This header does not have to be used, and any html with modal-header as
 * its selector will be picked up by ModalwidgetComponent.
 */

@Component({
  selector: 'app-modalheader',
  templateUrl: './modalheader.component.html',
  styleUrls: ['./modalheader.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class ModalheaderComponent implements OnInit {

  @Input() icon: any;
  @Input() title: String;
  @Input() subtitle: String;
  @Input() id: String;

  @Output() cancelClick: EventEmitter<any> = new EventEmitter();

  constructor() {
  }

  ngOnInit() {
    if (!this.id) {
      this.id = 'dashboard-header-' + Utility.s4();
    }
    if (this.icon == null) {
      this.icon = {margin: {}};
    }
  }

  onHeaderCancelClick(event) {
    this.cancelClick.emit(event);
  }
}
