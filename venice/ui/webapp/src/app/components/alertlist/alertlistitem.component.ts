import { Component, EventEmitter, Input, OnChanges, OnDestroy, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-alertlistitem',
  templateUrl: './alertlistitem.component.html',
  styleUrls: ['./alertlistitem.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertlistitemComponent implements OnInit, OnDestroy, OnChanges {

  @Input() data;

  @Output() alertClick: EventEmitter<any> = new EventEmitter();


  protected alert;
  constructor() { }

  ngOnInit() {
  }

  ngOnDestroy() {
  }

  ngOnChanges() {
    this.alert = this.data;
  }

  getAlertItemIconClass() {
    return 'alertlistitem-icon-' + this.alert.severity;
  }

  getAlertItemIconText(): string {
    return Utility.makeFirstLetterUppercase(this.alert.source, true);
  }

  getAlertItemIconTooltip(): string {
    return this.alert.source;
  }

  onAlertItemClick($event) {
    this.alert.check = !this.alert.check;
    this.alertClick.emit(this.alert);
  }

}
