import { Component, EventEmitter, Input, OnDestroy, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { BaseComponent } from '@app/components/base/base.component';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { GraphConfig } from '@app/models/frontend/shared/userpreference.interface';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-telemetrychartview',
  templateUrl: './telemetrychartview.component.html',
  styleUrls: ['./telemetrychartview.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: [Animations]
})
export class TelemetrychartviewComponent extends BaseComponent implements OnInit, OnDestroy {
  @Input() chartConfig: GraphConfig;
  @Input() inEditMode: boolean = false;
  @Input() hideEditButtons: boolean = false;
  @Input() selectedTimeRange: TimeRange;
  @Output() editReq: EventEmitter<any> = new EventEmitter<any>();
  @Output() deleteReq: EventEmitter<any> = new EventEmitter<any>();

  themeColor: string = '#b592e3';
  title: string = '';

  icon: Icon = {
    margin: {
      top: '8px',
      left: '0px'
    },
    matIcon: 'insert_chart_outlined'
  };

  // TODO: Support exporting data
  menuItems = [
    // {
    //   text: 'Flip card', onClick: () => {
    //     this.toggleFlip();
    //   }
    // },
    // {
    //   text: 'Export', onClick: () => {
    //     this.export();
    //   }
    // }
  ];

  subscriptions: Subscription[] = [];

  constructor(protected controllerService: ControllerService,
    protected clusterService: ClusterService,
    protected authService: AuthService,
    protected telemetryqueryService: MetricsqueryService) {
      super(controllerService);
  }

  ngOnInit() {
    if (this.chartConfig != null) {
      const titleTransform = this.chartConfig.graphTransforms.transforms['GraphTitle'];
      if (titleTransform != null && titleTransform.title != null) {
        this.title = titleTransform.title;
      }
    }
  }

  editClick() {
    this.editReq.emit();
  }

  deleteClick() {
    this.deleteReq.emit();
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

}
