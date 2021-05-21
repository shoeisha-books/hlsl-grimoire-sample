
class IShaderResource {
public:
	virtual ~IShaderResource()
	{
	}
	/// <summary>
	/// SRVÇ…ìoò^ÅB
	/// </summary>
	/// <param name="descriptorHandle"></param>
	virtual void RegistShaderResourceView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo) = 0;

};