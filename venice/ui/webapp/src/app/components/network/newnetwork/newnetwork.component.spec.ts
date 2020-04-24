import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, async } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { MonitoringGroupModule } from '@app/components/monitoring-group/monitoring-group.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { AuthService } from '@app/services/auth.service';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { NetworkService } from '@app/services/generated/network.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import * as _ from 'lodash';
import { configureTestSuite } from 'ng-bullet';
import { ConfirmationService } from 'primeng/primeng';
import { OrchestrationService } from '@app/services/generated/orchestration.service';
import { NewnetworkComponent } from './newnetwork.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';


describe('NewnetworkComponent', () => {
  let component: NewnetworkComponent;
  let fixture: ComponentFixture<NewnetworkComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [NewnetworkComponent],
      imports: [
        MonitoringGroupModule,
        SharedModule,
        ReactiveFormsModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FormsModule,
        NoopAnimationsModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        NetworkService,
        OrchestrationService,
        MessageService,
        ClusterService
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(NewnetworkComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
