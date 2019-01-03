import { Component, OnInit, Input, OnDestroy } from '@angular/core';
import { Subscription } from 'rxjs';
import { Table } from 'primeng/table';

@Component({
  selector: 'app-sorticon',
  templateUrl: './sorticon.component.html',
  styleUrls: ['./sorticon.component.scss']
})
export class SorticonComponent implements OnInit, OnDestroy {
  @Input() isSortable: boolean;
  @Input() field: string;

  subscription: Subscription;

  sortOrder: number;

  constructor(public dt: Table) {
    this.subscription = this.dt.tableService.sortSource$.subscribe(sortMeta => {
      this.updateSortState();
    });
  }

  ngOnInit() {
    this.updateSortState();
  }

  updateSortState() {
    if (this.dt.sortMode === 'single') {
      this.sortOrder = this.dt.isSorted(this.field) ? this.dt.sortOrder : 0;
    } else if (this.dt.sortMode === 'multiple') {
      const sortMeta = this.dt.getSortMeta(this.field);
      this.sortOrder = sortMeta ? sortMeta.order : 0;
    }
  }

  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }
}
