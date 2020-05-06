import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { FormsModule } from '@angular/forms';
import { ConfirmationService } from 'primeng/primeng';
import { ControllerService } from '@app/services/controller.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { NaplesdetailchartsComponent } from './naplesdetailcharts.component';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { NetworkService } from '@app/services/generated/network.service';
import { AuthService } from '@app/services/auth.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { MatIconRegistry } from '@angular/material';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';

describe('NaplesdetailchartsComponent', () => {
  let component: NaplesdetailchartsComponent;
  let fixture: ComponentFixture<NaplesdetailchartsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaplesdetailchartsComponent ],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule,
        NoopAnimationsModule,
        MaterialdesignModule,
        SharedModule,
        PrimengModule,
        FormsModule,
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        MessageService,
        ClusterService,
        NetworkService,
        MetricsqueryService,
        AuthService,
        AuthServiceGen,
        MonitoringService,
        MessageService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesdetailchartsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
