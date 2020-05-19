import { Component, Input, OnInit, OnChanges, SimpleChanges, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';
import { WorkloadNameInterface, WorkloadUtility } from '@app/common/WorkloadUtility';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';

@Component({
  selector: 'app-workloadscolumn',
  templateUrl: './workloadscolumn.component.html',
  styleUrls: ['./workloadscolumn.component.scss'],
  changeDetection: ChangeDetectionStrategy.OnPush,
})

export class WorkloadsColumnComponent implements OnChanges {
  @Input() workloads: WorkloadWorkload[] = null;
  @Input() hovered: boolean = false;
  @Input() numOfRows: number = 4;
  @Input() filterValue: string = null;
  @Input() linkTooltip: string = null;

  workloadNames: WorkloadNameInterface[] = [];


  ngOnChanges(changes: SimpleChanges) {
    if (changes.workloads) {
      this.workloadNames = WorkloadUtility.getWorkloadNames(this.workloads);
    }
  }

  getWorkloadFullnames(): string[] {
    if (!this.workloadNames) {
      return [];
    }
    return this.workloadNames.map((workload: WorkloadNameInterface) => workload.fullname);
  }

  encodeURL(url: string): string {
    return encodeURIComponent(url);
  }
}
