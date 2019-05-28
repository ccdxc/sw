import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { NaplesComponent } from './naples.component';
import { Component } from '@angular/core';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MatIconRegistry } from '@angular/material';
import { By } from '@angular/platform-browser';
import { TestingUtility } from '@app/common/TestingUtility';
import { ClusterSmartNIC, ClusterSmartNICStatus_admission_phase_uihint } from '@sdk/v1/models/generated/cluster';
import { MessageService } from '@app/services/message.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { AuthService } from '@app/services/auth.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  template: ''
})
class DummyComponent { }

describe('NaplesComponent', () => {
  let component: NaplesComponent;
  let fixture: ComponentFixture<NaplesComponent>;

  const naples1 = {
    'meta': {
      'name': 'naples1',
      'labels': {
        'Location': 'us-west-A'
      },
      'mod-time': '2018-08-23T17:35:08.534909931Z',
      'creation-time': '2018-08-23T17:30:08.534909931Z'
    },
    'spec': {
      'hostname': 'naples1-host'
    },
    'status': {
      'ip-config': {
        'ip-address': '0.0.0.0/0'
      },
      'primary-mac': '00ae.cd00.1142',
      'admission-phase': 'PENDING',
      'smartNicVersion': '1.0E',
    }
  };

  const naples2 = {
    'meta': {
      'name': 'naples2',
      'labels': {
        'Location': 'us-east-A'
      },
      'mod-time': '2018-08-23T17:25:08.534909931Z',
      'creation-time': '2018-08-23T17:20:08.534909931Z'
    },
    'spec': {
      'hostname': 'naples2-host'
    },
    'status': {
      'ip-config': {
        'ip-address': '0.0.0.10'
      },
      'primary-mac': '00ae.cd00.1143',
      'admission-phase': 'ADMITTED',
      'smartNicVersion': '1.0E',
    }
  };

  const naples3 = {
    'meta': {
      'name': 'naples2',
      'labels': {
        'Location': 'us-east-A'
      },
      'mod-time': '2018-08-23T17:15:08.534909931Z',
      'creation-time': '2018-08-23T17:20:08.534909931Z'
    },
    'spec': {
      'hostname': 'naples2-host'
    },
    'status': {
      'ip-config': {
        'ip-address': '0.0.0.10'
      },
      'primary-mac': '00ae.cd00.1143',
      'admission-phase': 'REJECTED',
      'smartNicVersion': '1.0E',
    }
  };

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        NaplesComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        HttpClientTestingModule,
        NoopAnimationsModule,
        SharedModule,
        MaterialdesignModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        ClusterService,
        MatIconRegistry,
        MetricsqueryService,
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesComponent);
    component = fixture.componentInstance;
    const service = TestBed.get(ClusterService);
    spyOn(service, 'WatchSmartNIC').and.returnValue(
      TestingUtility.createWatchEvents([naples1, naples2, naples3])
    );
  });

  it('should populate table', () => {
    TestingUtility.setAllPermissions();
    fixture.detectChanges();
    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Naples (3)');
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('.ui-table-scrollable-body tbody'));
    expect(tableBody).toBeTruthy();
    const caseMap = {
      'status.admission-phase': (field, rowData, rowIndex) => {
        expect(field.nativeElement.textContent).toContain(ClusterSmartNICStatus_admission_phase_uihint[rowData.status['admission-phase']]);

        if (rowData.status['admission-phase'] === 'REJECTED' || rowData.status['admission-phase'] === 'PENDING' ) {
          expect(field.children.length).toBe(2);
          if (rowData.status['admission-phase'] === 'REJECTED') {
            expect(field.children[1].nativeElement.textContent).toContain('error');
          }
          if (rowData.status['admission-phase'] === 'PENDING') {
            expect(field.children[1].nativeElement.textContent).toContain('notifications');
          }
        }
      },
    };
    TestingUtility.verifyTable([new ClusterSmartNIC(naples1), new ClusterSmartNIC(naples2), new ClusterSmartNIC(naples3)], component.cols, tableBody);
  });

  describe('RBAC', () => {
    it('metrics permission', () => {
      TestingUtility.addPermissions(
        [UIRolePermissions.metricsquery_read]
      );
    fixture.detectChanges();
      // metrics should be visible
      const cards = fixture.debugElement.queryAll(By.css('app-herocard'));
      expect(cards.length).toBe(3);
    });

    it('no permission', () => {
    fixture.detectChanges();
      // metrics should be hidden
      const cards = fixture.debugElement.queryAll(By.css('app-herocard'));
      expect(cards.length).toBe(0);
    });

  });

});
