import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { PrimengModule } from '@app/lib/primeng.module';

import { NaplesComponent } from './naples.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { ClusterService } from '@app/services/generated/cluster.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { By } from '@angular/platform-browser';

describe('NaplesComponent', () => {
  let component: NaplesComponent;
  let fixture: ComponentFixture<NaplesComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [NaplesComponent],
      imports: [
        PrimengModule,
        RouterTestingModule,
        HttpClientTestingModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        ClusterService,
        LogService,
        LogPublishersService,
        MessageService,
        MetricsqueryService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  describe('RBAC', () => {
    it('no permission', () => {
      fixture.detectChanges();
      // mat menu icon should be hidden
      // toggleFlip shouldn't flip state
      const menuItem = fixture.debugElement.queryAll(By.css('mat-menu'));
      expect(menuItem.length).toBe(0);
      const currState = component.flipState;
      component.toggleFlip();
      expect(currState).toEqual(component.flipState);
    });

  });
});
