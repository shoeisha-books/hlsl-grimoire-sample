
class IUnorderAccessResrouce : public IShaderResource {
public:
	virtual ~IUnorderAccessResrouce() {};
	/// <summary>
	/// UAVに登録。
	/// </summary>
	/// <param name=""></param>
	/// <param name="bufferNo"></param>
	virtual void RegistUnorderAccessView(D3D12_CPU_DESCRIPTOR_HANDLE, int bufferNo) = 0;
};