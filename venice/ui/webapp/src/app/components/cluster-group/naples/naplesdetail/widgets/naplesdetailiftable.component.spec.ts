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
import { NaplesdetailIftableComponent } from './naplesdetailiftable.component';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { AuthService } from '@app/services/auth.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { MatIconRegistry } from '@angular/material';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';

describe('NaplesdetailIftableComponent', () => {
  let component: NaplesdetailIftableComponent;
  let fixture: ComponentFixture<NaplesdetailIftableComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaplesdetailIftableComponent ],
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
        MessageService,
        ClusterService,
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
    fixture = TestBed.createComponent(NaplesdetailIftableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
