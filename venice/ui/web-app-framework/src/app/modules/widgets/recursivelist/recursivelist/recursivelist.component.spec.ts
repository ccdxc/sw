import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { RecursivelistComponent } from './recursivelist.component';

describe('RecursivelistComponent', () => {
  let component: RecursivelistComponent;
  let fixture: ComponentFixture<RecursivelistComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [RecursivelistComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(RecursivelistComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
