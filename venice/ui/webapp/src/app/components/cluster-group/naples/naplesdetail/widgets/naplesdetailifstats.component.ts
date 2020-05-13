import { Component, OnInit, ViewEncapsulation, Input, EventEmitter, Output } from '@angular/core';
import { InterfaceStats } from '../naplesdetail.component';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-naplesdetail-ifstats',
  templateUrl: 'naplesdetailifstats.component.html',
  styleUrls: ['naplesdetailifstats.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class NaplesdetailIfstatsComponent implements OnInit {
  @Input() interfaceStats: InterfaceStats[];
  @Input() mouseOverInterface: string = '';

  @Output() collapseExpandClickEmit: EventEmitter<string> = new EventEmitter();

  constructor() { }

  ngOnInit() {
  }

  getStats() {
    let ifs: InterfaceStats = null;
    if (this.interfaceStats && this.mouseOverInterface) {
      ifs = this.interfaceStats.find(item => item.ifname === this.mouseOverInterface);
    }
    return ifs;
  }

  expandInterfaceTable() {
    this.collapseExpandClickEmit.emit('expand');
  }

  displayStatsNumber (bytes: number): string {
    if (bytes === -1) {
      return '';
    }

    if (bytes === -1000) {
      return 'N/A';
    }

    return Utility.formatBytes(bytes, 2);
  }

  displayStatsCount (count: number): string {
    if (count === -1) {
      return '';
    }

    if (count === -1000) {
      return 'N/A';
    }

    return count.toString();
  }
}

